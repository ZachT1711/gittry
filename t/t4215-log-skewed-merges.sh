#!/bin/sh

test_description='git log --graph of skewed merges'

. ./test-lib.sh

test_expect_success 'log --graph with merge fusing with its left and right neighbors' '
	cat >expect <<-\EOF &&
	*   H
	|\
	| *   G
	| |\
	| | * F
	| * | E
	|/|\|
	| | * D
	| * | C
	| |/
	* / B
	|/
	* A
	EOF

	git checkout --orphan _p &&
	test_commit A &&
	test_commit B &&
	git checkout -b _q @^ && test_commit C &&
	git checkout -b _r @^ && test_commit D &&
	git checkout _p && git merge --no-ff _q _r -m E &&
	git checkout _r && test_commit F &&
	git checkout _p && git merge --no-ff _r -m G &&
	git checkout @^^ && git merge --no-ff _p -m H &&

	git log --graph --pretty=tformat:%s | sed "s/ *$//" >actual &&
	test_cmp expect actual
'

test_expect_success 'log --graph with left-skewed merge' '
	cat >expect <<-\EOF &&
	*-----.   0_H
	|\ \ \ \
	| | | | * 0_G
	| |_|_|/|
	|/| | | |
	| | | * | 0_F
	| |_|/|\|
	|/| | | |
	| | | | * 0_E
	| |_|_|/
	|/| | |
	| | * | 0_D
	| | |/
	| | * 0_C
	| |/
	|/|
	| * 0_B
	|/
	* 0_A
	EOF

	git checkout --orphan 0_p && test_commit 0_A &&
	git checkout -b 0_q 0_p && test_commit 0_B &&
	git checkout -b 0_r 0_p &&
	test_commit 0_C &&
	test_commit 0_D &&
	git checkout -b 0_s 0_p && test_commit 0_E &&
	git checkout -b 0_t 0_p && git merge --no-ff 0_r^ 0_s -m 0_F &&
	git checkout 0_p && git merge --no-ff 0_s -m 0_G &&
	git checkout @^ && git merge --no-ff 0_q 0_r 0_t 0_p -m 0_H &&

	git log --graph --pretty=tformat:%s | sed "s/ *$//" >actual &&
	test_cmp expect actual
'

test_expect_success 'log --graph with nested left-skewed merge' '
	cat >expect <<-\EOF &&
	*   1_H
	|\
	| *   1_G
	| |\
	| | * 1_F
	| * | 1_E
	|/| |
	| * | 1_D
	* | | 1_C
	|/ /
	* / 1_B
	|/
	* 1_A
	EOF

	git checkout --orphan 1_p &&
	test_commit 1_A &&
	test_commit 1_B &&
	test_commit 1_C &&
	git checkout -b 1_q @^ && test_commit 1_D &&
	git checkout 1_p && git merge --no-ff 1_q -m 1_E &&
	git checkout -b 1_r @~3 && test_commit 1_F &&
	git checkout 1_p && git merge --no-ff 1_r -m 1_G &&
	git checkout @^^ && git merge --no-ff 1_p -m 1_H &&

	git log --graph --pretty=tformat:%s | sed "s/ *$//" >actual &&
	test_cmp expect actual
'

test_expect_success 'log --graph with nested left-skewed merge following normal merge' '
	cat >expect <<-\EOF &&
	*   2_K
	|\
	| *   2_J
	| |\
	| | *   2_H
	| | |\
	| | * | 2_G
	| |/| |
	| | * | 2_F
	| * | | 2_E
	| |/ /
	| * | 2_D
	* | | 2_C
	| |/
	|/|
	* | 2_B
	|/
	* 2_A
	EOF

	git checkout --orphan 2_p &&
	test_commit 2_A &&
	test_commit 2_B &&
	test_commit 2_C &&
	git checkout -b 2_q @^^ &&
	test_commit 2_D &&
	test_commit 2_E &&
	git checkout -b 2_r @^ && test_commit 2_F &&
	git checkout 2_q &&
	git merge --no-ff 2_r -m 2_G &&
	git merge --no-ff 2_p^ -m 2_H &&
	git checkout -b 2_s @^^ && git merge --no-ff 2_q -m 2_J &&
	git checkout 2_p && git merge --no-ff 2_s -m 2_K &&

	git log --graph --pretty=tformat:%s | sed "s/ *$//" >actual &&
	test_cmp expect actual
'

test_expect_success 'log --graph with nested right-skewed merge following left-skewed merge' '
	cat >expect <<-\EOF &&
	*   3_J
	|\
	| *   3_H
	| |\
	| | * 3_G
	| * | 3_F
	|/| |
	| * | 3_E
	| |\|
	| | * 3_D
	| * | 3_C
	| |/
	| * 3_B
	|/
	* 3_A
	EOF

	git checkout --orphan 3_p &&
	test_commit 3_A &&
	git checkout -b 3_q &&
	test_commit 3_B &&
	test_commit 3_C &&
	git checkout -b 3_r @^ &&
	test_commit 3_D &&
	git checkout 3_q && git merge --no-ff 3_r -m 3_E &&
	git checkout 3_p && git merge --no-ff 3_q -m 3_F &&
	git checkout 3_r && test_commit 3_G &&
	git checkout 3_p && git merge --no-ff 3_r -m 3_H &&
	git checkout @^^ && git merge --no-ff 3_p -m 3_J &&

	git log --graph --pretty=tformat:%s | sed "s/ *$//" >actual &&
	test_cmp expect actual
'

test_expect_success 'log --graph with right-skewed merge following a left-skewed one' '
	cat >expect <<-\EOF &&
	*   4_H
	|\
	| *   4_G
	| |\
	| * | 4_F
	|/| |
	| * |   4_E
	| |\ \
	| | * | 4_D
	| |/ /
	|/| |
	| | * 4_C
	| |/
	| * 4_B
	|/
	* 4_A
	EOF

	git checkout --orphan 4_p &&
	test_commit 4_A &&
	test_commit 4_B &&
	test_commit 4_C &&
	git checkout -b 4_q @^^ && test_commit 4_D &&
	git checkout -b 4_r 4_p^ && git merge --no-ff 4_q -m 4_E &&
	git checkout -b 4_s 4_p^^ &&
	git merge --no-ff 4_r -m 4_F &&
	git merge --no-ff 4_p -m 4_G &&
	git checkout @^^ && git merge --no-ff 4_s -m 4_H &&

	git log --graph --date-order --pretty=tformat:%s | sed "s/ *$//" >actual &&
	test_cmp expect actual
'

test_expect_success 'log --graph with octopus merge with column joining its penultimate parent' '
	cat >expect <<-\EOF &&
	*   5_H
	|\
	| *-.   5_G
	| |\ \
	| | | * 5_F
	| | * |   5_E
	| |/|\ \
	| |_|/ /
	|/| | /
	| | |/
	* | | 5_D
	| | * 5_C
	| |/
	|/|
	| * 5_B
	|/
	* 5_A
	EOF

	git checkout --orphan 5_p &&
	test_commit 5_A &&
	git branch 5_q &&
	git branch 5_r &&
	test_commit 5_B &&
	git checkout 5_q && test_commit 5_C &&
	git checkout 5_r && test_commit 5_D &&
	git checkout 5_p &&
	git merge --no-ff 5_q 5_r -m 5_E &&
	git checkout 5_q && test_commit 5_F &&
	git checkout -b 5_s 5_p^ &&
	git merge --no-ff 5_p 5_q -m 5_G &&
	git checkout 5_r &&
	git merge --no-ff 5_s -m 5_H &&

	git log --graph --pretty=tformat:%s | sed "s/ *$//" >actual &&
	test_cmp expect actual
'

test_done
