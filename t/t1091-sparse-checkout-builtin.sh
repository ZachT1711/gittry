#!/bin/sh

test_description='sparse checkout builtin tests'

. ./test-lib.sh

test_expect_success 'setup' '
	git init repo &&
	(
		cd repo &&
		echo "initial" >a &&
		mkdir folder1 folder2 deep &&
		mkdir deep/deeper1 deep/deeper2 &&
		mkdir deep/deeper1/deepest &&
		cp a folder1 &&
		cp a folder2 &&
		cp a deep &&
		cp a deep/deeper1 &&
		cp a deep/deeper2 &&
		cp a deep/deeper1/deepest &&
		git add . &&
		git commit -m "initial commit"
	)
'

test_expect_success 'git sparse-checkout list (empty)' '
	git -C repo sparse-checkout list >list 2>err &&
	test_must_be_empty list &&
	test_i18ngrep "this worktree is not sparse (sparse-checkout file may not exist)" err
'

test_expect_success 'git sparse-checkout list (populated)' '
	test_when_finished rm -f repo/.git/info/sparse-checkout &&
	cat >repo/.git/info/sparse-checkout <<-EOF &&
		/folder1/*
		/deep/
		**/a
		!*bin*
	EOF
	cp repo/.git/info/sparse-checkout expect &&
	git -C repo sparse-checkout list >list &&
	test_cmp expect list
'

test_expect_success 'git sparse-checkout init' '
	git -C repo sparse-checkout init &&
	cat >expect <<-EOF &&
		/*
		!/*/
	EOF
	test_cmp expect repo/.git/info/sparse-checkout &&
	git -C repo config --list >config &&
	test_i18ngrep "core.sparsecheckout=true" config &&
	ls repo >dir  &&
	echo a >expect &&
	test_cmp expect dir
'

test_expect_success 'git sparse-checkout list after init' '
	git -C repo sparse-checkout list >actual &&
	cat >expect <<-EOF &&
		/*
		!/*/
	EOF
	test_cmp expect actual
'

test_expect_success 'init with existing sparse-checkout' '
	echo "*folder*" >> repo/.git/info/sparse-checkout &&
	git -C repo sparse-checkout init &&
	cat >expect <<-EOF &&
		/*
		!/*/
		*folder*
	EOF
	test_cmp expect repo/.git/info/sparse-checkout &&
	ls repo >dir  &&
	cat >expect <<-EOF &&
		a
		folder1
		folder2
	EOF
	test_cmp expect dir
'

test_expect_success 'clone --sparse' '
	git clone --sparse repo clone &&
	git -C clone sparse-checkout list >actual &&
	cat >expect <<-EOF &&
		/*
		!/*/
	EOF
	test_cmp expect actual &&
	ls clone >dir &&
	echo a >expect &&
	test_cmp expect dir
'

test_expect_success 'set enables config' '
	git init empty-config &&
	(
		cd empty-config &&
		test_commit test file &&
		test_path_is_missing .git/config.worktree &&
		test_must_fail git sparse-checkout set nothing &&
		test_i18ngrep "sparseCheckout = false" .git/config.worktree &&
		git sparse-checkout set "/*" &&
		test_i18ngrep "sparseCheckout = true" .git/config.worktree
	)
'

test_expect_success 'set sparse-checkout using builtin' '
	git -C repo sparse-checkout set "/*" "!/*/" "*folder*" &&
	cat >expect <<-EOF &&
		/*
		!/*/
		*folder*
	EOF
	git -C repo sparse-checkout list >actual &&
	test_cmp expect actual &&
	test_cmp expect repo/.git/info/sparse-checkout &&
	ls repo >dir  &&
	cat >expect <<-EOF &&
		a
		folder1
		folder2
	EOF
	test_cmp expect dir
'

test_expect_success 'set sparse-checkout using --stdin' '
	cat >expect <<-EOF &&
		/*
		!/*/
		/folder1/
		/folder2/
	EOF
	git -C repo sparse-checkout set --stdin <expect &&
	git -C repo sparse-checkout list >actual &&
	test_cmp expect actual &&
	test_cmp expect repo/.git/info/sparse-checkout &&
	ls repo >dir  &&
	cat >expect <<-EOF &&
		a
		folder1
		folder2
	EOF
	test_cmp expect dir
'

test_expect_success 'cone mode: match patterns' '
	git -C repo config --worktree core.sparseCheckoutCone true &&
	rm -rf repo/a repo/folder1 repo/folder2 &&
	git -C repo read-tree -mu HEAD 2>err &&
	test_i18ngrep ! "disabling cone patterns" err &&
	git -C repo reset --hard &&
	ls repo >dir  &&
	cat >expect <<-EOF &&
		a
		folder1
		folder2
	EOF
	test_cmp expect dir
'

test_expect_success 'cone mode: warn on bad pattern' '
	test_when_finished mv sparse-checkout repo/.git/info/ &&
	cp repo/.git/info/sparse-checkout . &&
	echo "!/deep/deeper/*" >>repo/.git/info/sparse-checkout &&
	git -C repo read-tree -mu HEAD 2>err &&
	test_i18ngrep "unrecognized negative pattern" err
'

test_expect_success 'sparse-checkout disable' '
	git -C repo sparse-checkout disable &&
	test_path_is_missing repo/.git/info/sparse-checkout &&
	git -C repo config --list >config &&
	test_i18ngrep "core.sparsecheckout=false" config &&
	ls repo >dir &&
	cat >expect <<-EOF &&
		a
		deep
		folder1
		folder2
	EOF
	test_cmp expect dir
'

test_expect_success 'cone mode: init and set' '
	git -C repo sparse-checkout init --cone &&
	git -C repo config --list >config &&
	test_i18ngrep "core.sparsecheckoutcone=true" config &&
	ls repo >dir  &&
	echo a >expect &&
	test_cmp expect dir &&
	git -C repo sparse-checkout set deep/deeper1/deepest/ 2>err &&
	test_line_count = 0 err &&
	ls repo >dir  &&
	cat >expect <<-EOF &&
		a
		deep
	EOF
	test_cmp expect dir &&
	ls repo/deep >dir  &&
	cat >expect <<-EOF &&
		a
		deeper1
	EOF
	test_cmp expect dir &&
	ls repo/deep/deeper1 >dir  &&
	cat >expect <<-EOF &&
		a
		deepest
	EOF
	test_cmp expect dir &&
	cat >expect <<-EOF &&
		/*
		!/*/
		/deep/
		!/deep/*/
		/deep/deeper1/
		!/deep/deeper1/*/
		/deep/deeper1/deepest/
	EOF
	test_cmp expect repo/.git/info/sparse-checkout &&
	git -C repo sparse-checkout set --stdin 2>err <<-EOF &&
		folder1
		folder2
	EOF
	test_line_count = 0 err &&
	cat >expect <<-EOF &&
		a
		folder1
		folder2
	EOF
	ls repo >dir &&
	test_cmp expect dir
'

test_expect_success 'cone mode: set with nested folders' '
	git -C repo sparse-checkout set deep deep/deeper1/deepest 2>err &&
	test_line_count = 0 err &&
	cat >expect <<-EOF &&
		/*
		!/*/
		/deep/
	EOF
	test_cmp repo/.git/info/sparse-checkout expect
'

test_expect_success 'revert to old sparse-checkout on bad update' '
	echo update >repo/deep/deeper2/a &&
	cp repo/.git/info/sparse-checkout expect &&
	test_must_fail git -C repo sparse-checkout set deep/deeper1 2>err &&
	test_i18ngrep "Cannot update sparse checkout" err &&
	test_cmp repo/.git/info/sparse-checkout expect &&
	ls repo/deep >dir &&
	cat >expect <<-EOF &&
		a
		deeper1
		deeper2
	EOF
	test_cmp dir expect
'

test_expect_success 'revert to old sparse-checkout on empty update' '
	git init empty-test &&
	(
		echo >file &&
		git add file &&
		git commit -m "test" &&
		test_must_fail git sparse-checkout set nothing 2>err &&
		test_i18ngrep "Sparse checkout leaves no entry on working directory" err &&
		test_i18ngrep ! ".git/index.lock" err &&
		git sparse-checkout set file
	)
'

test_expect_success 'fail when lock is taken' '
	test_when_finished rm -rf repo/.git/info/sparse-checkout.lock &&
	touch repo/.git/info/sparse-checkout.lock &&
	test_must_fail git -C repo sparse-checkout set deep 2>err &&
	test_i18ngrep "File exists" err
'

test_expect_success '.gitignore should not warn about cone mode' '
	git -C repo config --worktree core.sparseCheckoutCone true &&
	echo "**/bin/*" >repo/.gitignore &&
	git -C repo reset --hard 2>err &&
	test_i18ngrep ! "disabling cone patterns" err
'

test_done
