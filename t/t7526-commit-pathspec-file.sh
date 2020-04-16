#!/bin/sh

test_description='commit --pathspec-from-file'

. ./test-lib.sh

test_tick

test_expect_success setup '
	test_commit file0 &&
	git tag checkpoint &&

	echo A >fileA.t &&
	echo B >fileB.t &&
	echo C >fileC.t &&
	echo D >fileD.t &&
	git add fileA.t fileB.t fileC.t fileD.t
'

restore_checkpoint () {
	git reset --soft checkpoint
}

verify_expect () {
	git diff-tree --no-commit-id --name-status -r HEAD >actual &&
	test_cmp expect actual
}

test_expect_success '--pathspec-from-file from stdin' '
	restore_checkpoint &&

	echo fileA.t | git commit --pathspec-from-file=- -m "Commit" &&

	cat >expect <<-\EOF &&
	A	fileA.t
	EOF
	verify_expect
'

test_expect_success '--pathspec-from-file from file' '
	restore_checkpoint &&

	echo fileA.t >list &&
	git commit --pathspec-from-file=list -m "Commit" &&

	cat >expect <<-\EOF &&
	A	fileA.t
	EOF
	verify_expect
'

test_expect_success 'NUL delimiters' '
	restore_checkpoint &&

	printf "fileA.t\0fileB.t\0" | git commit --pathspec-from-file=- --pathspec-file-nul -m "Commit" &&

	cat >expect <<-\EOF &&
	A	fileA.t
	A	fileB.t
	EOF
	verify_expect
'

test_expect_success 'LF delimiters' '
	restore_checkpoint &&

	printf "fileA.t\nfileB.t\n" | git commit --pathspec-from-file=- -m "Commit" &&

	cat >expect <<-\EOF &&
	A	fileA.t
	A	fileB.t
	EOF
	verify_expect
'

test_expect_success 'no trailing delimiter' '
	restore_checkpoint &&

	printf "fileA.t\nfileB.t" | git commit --pathspec-from-file=- -m "Commit" &&

	cat >expect <<-\EOF &&
	A	fileA.t
	A	fileB.t
	EOF
	verify_expect
'

test_expect_success 'CRLF delimiters' '
	restore_checkpoint &&

	printf "fileA.t\r\nfileB.t\r\n" | git commit --pathspec-from-file=- -m "Commit" &&

	cat >expect <<-\EOF &&
	A	fileA.t
	A	fileB.t
	EOF
	verify_expect
'

test_expect_success 'quotes' '
	restore_checkpoint &&

	printf "\"file\\101.t\"" | git commit --pathspec-from-file=- -m "Commit" &&

	cat >expect <<-\EOF &&
	A	fileA.t
	EOF
	verify_expect expect
'

test_expect_success 'quotes not compatible with --pathspec-file-nul' '
	restore_checkpoint &&

	printf "\"file\\101.t\"" >list &&
	test_must_fail git commit --pathspec-from-file=list --pathspec-file-nul -m "Commit"
'

test_expect_success 'only touches what was listed' '
	restore_checkpoint &&

	printf "fileB.t\nfileC.t\n" | git commit --pathspec-from-file=- -m "Commit" &&

	cat >expect <<-\EOF &&
	A	fileB.t
	A	fileC.t
	EOF
	verify_expect
'

test_done
