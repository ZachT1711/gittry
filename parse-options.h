#ifndef PARSE_OPTIONS_H
#define PARSE_OPTIONS_H

/**
 * The parse-options API is used to parse and massage options in Git
 * and to provide a usage help with consistent look.
 *
 * Basics
 * ------
 *
 * The argument vector `argv[]` may usually contain mandatory or optional
 * 'non-option arguments', e.g. a filename or a branch, and 'options'.
 * Options are optional arguments that start with a dash and
 * that allow to change the behavior of a command.
 *
 * - There are basically three types of options:
 *   'boolean' options,
 *   options with (mandatory) 'arguments' and
 *   options with 'optional arguments'
 *   (i.e. a boolean option that can be adjusted).
 *
 * - There are basically two forms of options:
 *   'Short options' consist of one dash (`-`) and one alphanumeric
 *   character.
 *   'Long options' begin with two dashes (`--`) and some
 *   alphanumeric characters.
 *
 * - Options are case-sensitive.
 *   Please define 'lower-case long options' only.
 *
 * The parse-options API allows:
 *
 * - 'stuck' and 'separate form' of options with arguments.
 *   `-oArg` is stuck, `-o Arg` is separate form.
 *   `--option=Arg` is stuck, `--option Arg` is separate form.
 *
 * - Long options may be 'abbreviated', as long as the abbreviation
 *   is unambiguous.
 *
 * - Short options may be bundled, e.g. `-a -b` can be specified as `-ab`.
 *
 * - Boolean long options can be 'negated' (or 'unset') by prepending
 *   `no-`, e.g. `--no-abbrev` instead of `--abbrev`. Conversely,
 *   options that begin with `no-` can be 'negated' by removing it.
 *   Other long options can be unset (e.g., set string to NULL, set
 *   integer to 0) by prepending `no-`.
 *
 * - Options and non-option arguments can clearly be separated using the `--`
 *   option, e.g. `-a -b --option -- --this-is-a-file` indicates that
 *   `--this-is-a-file` must not be processed as an option.
 *
 * Steps to parse options
 * ----------------------
 *
 * - `#include "parse-options.h"`
 *
 * - define a NULL-terminated
 *   `static const char * const builtin_foo_usage[]` array
 *   containing alternative usage strings
 *
 * - define `builtin_foo_options` array as described below
 *   in section 'Data Structure'.
 *
 * - in `cmd_foo(int argc, const char **argv, const char *prefix)`
 *   call
 *
 * 	argc = parse_options(argc, argv, prefix, builtin_foo_options, builtin_foo_usage, flags);
 *
 * `parse_options()` will filter out the processed options of `argv[]` and leave the
 * non-option arguments in `argv[]`.
 * `argc` is updated appropriately because of the assignment.
 *
 * You can also pass NULL instead of a usage array as the fifth parameter of
 * parse_options(), to avoid displaying a help screen with usage info and
 * option list. This should only be done if necessary, e.g. to implement
 * a limited parser for only a subset of the options that needs to be run
 * before the full parser, which in turn shows the full help message.
 *
 * Sophisticated option parsing
 * ----------------------------
 *
 * If you need, for example, option callbacks with optional arguments
 * or without arguments at all, or if you need other special cases,
 * that are not handled by the macros here, you need to specify the
 * members of the `option` structure manually.
 *
 * Examples
 * --------
 *
 * See `test-parse-options.c` and
 * `builtin/add.c`,
 * `builtin/clone.c`,
 * `builtin/commit.c`,
 * `builtin/fetch.c`,
 * `builtin/fsck.c`,
 * `builtin/rm.c`
 * for real-world examples.
 *
 */

enum parse_opt_type {
	/* special types */
	OPTION_END,
	OPTION_ARGUMENT,
	OPTION_GROUP,
	OPTION_NUMBER,
	OPTION_ALIAS,
	/* options with no arguments */
	OPTION_BIT,
	OPTION_NEGBIT,
	OPTION_BITOP,
	OPTION_COUNTUP,
	OPTION_SET_INT,
	OPTION_CMDMODE,
	/* options with arguments (usually) */
	OPTION_STRING,
	OPTION_INTEGER,
	OPTION_MAGNITUDE,
	OPTION_CALLBACK,
	OPTION_LOWLEVEL_CALLBACK,
	OPTION_FILENAME
};

enum parse_opt_flags {

	/**
	 * Keep the `--` that usually separates options from non-option arguments.
	 */
	PARSE_OPT_KEEP_DASHDASH = 1,

	/**
	 * Usually the whole argument vector is massaged and reordered.
	 * Using this flag, processing is stopped at the first non-option
	 * argument.
	 */
	PARSE_OPT_STOP_AT_NON_OPTION = 2,

	/**
	 * Keep the first argument, which contains the program name.  It's
	 * removed from argv[] by default.
	 */
	PARSE_OPT_KEEP_ARGV0 = 4,

	/**
	 * Keep unknown arguments instead of erroring out.  This doesn't
	 * work for all combinations of arguments as users might expect
	 * it to do. E.g. if the first argument in `--unknown --known`
	 * takes a value (which we can't know), the second one is
	 * mistakenly interpreted as a known option.  Similarly, if
	 * `PARSE_OPT_STOP_AT_NON_OPTION` is set, the second argument in
	 * `--unknown value` will be mistakenly interpreted as a
	 * non-option, not as a value belonging to the unknown option,
	 * the parser early. That's why parse_options() errors out if
	 * both options are set.
	 */
	PARSE_OPT_KEEP_UNKNOWN = 8,

	/**
	 * By default, parse_options() handles `-h`, `--help` and
	 * `--help-all` internally, by showing a help screen. This option
	 * turns it off and allows one to add custom handlers for these
	 * options, or to just leave them unknown.
	 */
	PARSE_OPT_NO_INTERNAL_HELP = 16,

	PARSE_OPT_ONE_SHOT = 32
};

enum parse_opt_option_flags {
	PARSE_OPT_OPTARG  = 1,
	PARSE_OPT_NOARG   = 2,
	PARSE_OPT_NONEG   = 4,
	PARSE_OPT_HIDDEN  = 8,
	PARSE_OPT_LASTARG_DEFAULT = 16,
	PARSE_OPT_NODASH = 32,
	PARSE_OPT_LITERAL_ARGHELP = 64,
	PARSE_OPT_SHELL_EVAL = 256,
	PARSE_OPT_NOCOMPLETE = 512,
	PARSE_OPT_COMP_ARG = 1024
};

enum parse_opt_result {
	PARSE_OPT_COMPLETE = -3,
	PARSE_OPT_HELP = -2,
	PARSE_OPT_ERROR = -1,	/* must be the same as error() */
	PARSE_OPT_DONE = 0,	/* fixed so that "return 0" works */
	PARSE_OPT_NON_OPTION,
	PARSE_OPT_UNKNOWN
};

struct option;

/**
 * Option Callbacks
 * ----------------
 *
 * The function must be defined in this form:
 *
 * 	int func(const struct option *opt, const char *arg, int unset)
 *
 * The callback mechanism is as follows:
 *
 * - Inside `func`, the only interesting member of the structure
 *   given by `opt` is the void pointer `opt->value`.
 *   `*opt->value` will be the value that is saved into `var`, if you
 *   use `OPT_CALLBACK()`.
 *   For example, do `*(unsigned long *)opt->value = 42;` to get 42
 *   into an `unsigned long` variable.
 *
 * - Return value `0` indicates success and non-zero return
 *   value will invoke `usage_with_options()` and, thus, die.
 *
 * - If the user negates the option, `arg` is `NULL` and `unset` is 1.
 *
 */
typedef int parse_opt_cb(const struct option *, const char *arg, int unset);

struct parse_opt_ctx_t;
typedef enum parse_opt_result parse_opt_ll_cb(struct parse_opt_ctx_t *ctx,
					      const struct option *opt,
					      const char *arg, int unset);

/*
 * `type`::
 *   holds the type of the option, you must have an OPTION_END last in your
 *   array.
 *
 * `short_name`::
 *   the character to use as a short option name, '\0' if none.
 *
 * `long_name`::
 *   the long option name, without the leading dashes, NULL if none.
 *
 * `value`::
 *   stores pointers to the values to be filled.
 *
 * `argh`::
 *   token to explain the kind of argument this option wants. Keep it
 *   homogeneous across the repository. Should be wrapped by N_() for
 *   translation.
 *
 * `help`::
 *   the short help associated to what the option does.
 *   Must never be NULL (except for OPTION_END).
 *   OPTION_GROUP uses this pointer to store the group header.
 *   Should be wrapped by N_() for translation.
 *
 * `flags`::
 *   mask of parse_opt_option_flags.
 *   PARSE_OPT_OPTARG: says that the argument is optional (not for BOOLEANs)
 *   PARSE_OPT_NOARG: says that this option does not take an argument
 *   PARSE_OPT_NONEG: says that this option cannot be negated
 *   PARSE_OPT_HIDDEN: this option is skipped in the default usage, and
 *                     shown only in the full usage.
 *   PARSE_OPT_LASTARG_DEFAULT: says that this option will take the default
 *				value if no argument is given when the option
 *				is last on the command line. If the option is
 *				not last it will require an argument.
 *				Should not be used with PARSE_OPT_OPTARG.
 *   PARSE_OPT_NODASH: this option doesn't start with a dash.
 *   PARSE_OPT_LITERAL_ARGHELP: says that argh shouldn't be enclosed in brackets
 *				(i.e. '<argh>') in the help message.
 *				Useful for options with multiple parameters.
 *   PARSE_OPT_NOCOMPLETE: by default all visible options are completable
 *			   by git-completion.bash. This option suppresses that.
 *   PARSE_OPT_COMP_ARG: this option forces to git-completion.bash to
 *			 complete an option as --name= not --name even if
 *			 the option takes optional argument.
 *
 * `callback`::
 *   pointer to the callback to use for OPTION_CALLBACK
 *
 * `defval`::
 *   default value to fill (*->value) with for PARSE_OPT_OPTARG.
 *   OPTION_{BIT,SET_INT} store the {mask,integer} to put in the value when met.
 *   CALLBACKS can use it like they want.
 *
 * `ll_callback`::
 *   pointer to the callback to use for OPTION_LOWLEVEL_CALLBACK
 *
 */
struct option {
	enum parse_opt_type type;
	int short_name;
	const char *long_name;
	void *value;
	const char *argh;
	const char *help;

	int flags;
	parse_opt_cb *callback;
	intptr_t defval;
	parse_opt_ll_cb *ll_callback;
	intptr_t extra;
};

/**
 * Data Structure
 * --------------
 *
 * The main data structure is an array of the `option` struct, say
 * `static struct option builtin_add_options[]`.
 * The following macros can be used to easily define options.
 * The last element of the array must be `OPT_END()`.
 *
 * If not stated otherwise, interpret the arguments as follows:
 *
 * - `short` is a character for the short option
 *   (e.g. `'e'` for `-e`, use `0` to omit),
 *
 * - `long` is a string for the long option
 *   (e.g. `"example"` for `--example`, use `NULL` to omit),
 *
 * - `int_var` is an integer variable,
 *
 * - `str_var` is a string variable (`char *`),
 *
 * - `arg_str` is the string that is shown as argument
 *   (e.g. `"branch"` will result in `<branch>`).
 *   If set to `NULL`, three dots (`...`) will be displayed.
 *
 * - `description` is a short string to describe the effect of the option.
 *   It shall begin with a lower-case letter and a full stop (`.`) shall be
 *   omitted at the end.
 */

#define OPT_BIT_F(s, l, v, h, b, f) { OPTION_BIT, (s), (l), (v), NULL, (h), \
				      PARSE_OPT_NOARG|(f), NULL, (b) }
#define OPT_COUNTUP_F(s, l, v, h, f) { OPTION_COUNTUP, (s), (l), (v), NULL, \
				       (h), PARSE_OPT_NOARG|(f) }
#define OPT_SET_INT_F(s, l, v, h, i, f) { OPTION_SET_INT, (s), (l), (v), NULL, \
					  (h), PARSE_OPT_NOARG | (f), NULL, (i) }
#define OPT_BOOL_F(s, l, v, h, f)   OPT_SET_INT_F(s, l, v, h, 1, f)
#define OPT_CALLBACK_F(s, l, v, a, h, f, cb)			\
	{ OPTION_CALLBACK, (s), (l), (v), (a), (h), (f), (cb) }
#define OPT_STRING_F(s, l, v, a, h, f)   { OPTION_STRING,  (s), (l), (v), (a), (h), (f) }
#define OPT_INTEGER_F(s, l, v, h, f)     { OPTION_INTEGER, (s), (l), (v), N_("n"), (h), (f) }

#define OPT_END()                   { OPTION_END }

/**
 * Introduce a long-option argument that will be kept in `argv[]`.
 * If this option was seen, `int_var` will be set to one (except
 * if a `NULL` pointer was passed).
 */
#define OPT_ARGUMENT(l, v, h)       { OPTION_ARGUMENT, 0, (l), (v), NULL, \
				      (h), PARSE_OPT_NOARG, NULL, 1 }

/**
 * Start an option group. `description` is a short string that describes the
 * group or an empty string. Start the description with an upper-case letter.
 */
#define OPT_GROUP(h)                { OPTION_GROUP, 0, NULL, NULL, NULL, (h) }

/**
 * Introduce a boolean option. If used, `int_var` is bitwise-ored with `mask`.
 */
#define OPT_BIT(s, l, v, h, b)      OPT_BIT_F(s, l, v, h, b, 0)

#define OPT_BITOP(s, l, v, h, set, clear) { OPTION_BITOP, (s), (l), (v), NULL, (h), \
					    PARSE_OPT_NOARG|PARSE_OPT_NONEG, NULL, \
					    (set), NULL, (clear) }

/**
 * Introduce a boolean option. If used, `int_var` is bitwise-anded with the
 * inverted `mask`.
 */
#define OPT_NEGBIT(s, l, v, h, b)   { OPTION_NEGBIT, (s), (l), (v), NULL, \
				      (h), PARSE_OPT_NOARG, NULL, (b) }

/**
 * Introduce a count-up option.
 * Each use of `--option` increments `int_var`, starting from zero
 * (even if initially negative), and `--no-option` resets it to
 * zero. To determine if `--option` or `--no-option` was encountered at
 * all, initialize `int_var` to a negative value, and if it is still
 * negative after parse_options(), then neither `--option` nor
 * `--no-option` was seen.
 */
#define OPT_COUNTUP(s, l, v, h)     OPT_COUNTUP_F(s, l, v, h, 0)

/**
 * Introduce an integer option. `int_var` is set to `integer` with `--option`,
 * and reset to zero with `--no-option`.
 */
#define OPT_SET_INT(s, l, v, h, i)  OPT_SET_INT_F(s, l, v, h, i, 0)

/**
 * Introduce a boolean option. `int_var` is set to one with `--option` and set
 * to zero with `--no-option`.
 */
#define OPT_BOOL(s, l, v, h)        OPT_BOOL_F(s, l, v, h, 0)

#define OPT_HIDDEN_BOOL(s, l, v, h) { OPTION_SET_INT, (s), (l), (v), NULL, \
				      (h), PARSE_OPT_NOARG | PARSE_OPT_HIDDEN, NULL, 1}

/**
 * Define an "operation mode" option, only one of which in the same
 * group of "operating mode" options that share the same `int_var`
 * can be given by the user. `enum_val` is set to `int_var` when the
 * option is used, but an error is reported if other "operating mode"
 * option has already set its value to the same `int_var`.
 */
#define OPT_CMDMODE(s, l, v, h, i)  { OPTION_CMDMODE, (s), (l), (v), NULL, \
				      (h), PARSE_OPT_NOARG|PARSE_OPT_NONEG, NULL, (i) }

/**
 * Introduce an option with integer argument. The integer is put into `int_var`.
 */
#define OPT_INTEGER(s, l, v, h)     OPT_INTEGER_F(s, l, v, h, 0)

/**
 * Introduce an option with a size argument. The argument must be a
 * non-negative integer and may include a suffix of 'k', 'm' or 'g' to
 * scale the provided value by 1024, 1024^2 or 1024^3 respectively.
 * The scaled value is put into `unsigned_long_var`.
 */
#define OPT_MAGNITUDE(s, l, v, h)   { OPTION_MAGNITUDE, (s), (l), (v), \
				      N_("n"), (h), PARSE_OPT_NONEG }

/**
 * Introduce an option with string argument. The string argument is put
 * into `str_var`.
 */
#define OPT_STRING(s, l, v, a, h)   OPT_STRING_F(s, l, v, a, h, 0)

/**
 * Introduce an option with string argument.
 * The string argument is stored as an element in `string_list`.
 * Use of `--no-option` will clear the list of preceding values.
 */
#define OPT_STRING_LIST(s, l, v, a, h) \
				    { OPTION_CALLBACK, (s), (l), (v), (a), \
				      (h), 0, &parse_opt_string_list }

#define OPT_UYN(s, l, v, h)         { OPTION_CALLBACK, (s), (l), (v), NULL, \
				      (h), PARSE_OPT_NOARG, &parse_opt_tertiary }

/**
 * Introduce an option with expiry date argument, see `parse_expiry_date()`.
 * The timestamp is put into `timestamp_t_var`.
 */
#define OPT_EXPIRY_DATE(s, l, v, h) \
	{ OPTION_CALLBACK, (s), (l), (v), N_("expiry-date"),(h), 0,	\
	  parse_opt_expiry_date_cb }

/**
 * Introduce an option with argument.
 * The argument will be fed into the function given by `func_ptr`
 * and the result will be put into `var`.
 */
#define OPT_CALLBACK(s, l, v, a, h, f) OPT_CALLBACK_F(s, l, v, a, h, 0, f)

/**
 * Recognize numerical options like -123 and feed the integer as
 * if it was an argument to the function given by `func_ptr`.
 * The result will be put into `var`.  There can be only one such
 * option definition.  It cannot be negated and it takes no
 * arguments.  Short options that happen to be digits take
 * precedence over it.
 */
#define OPT_NUMBER_CALLBACK(v, h, f) \
	{ OPTION_NUMBER, 0, NULL, (v), NULL, (h), \
	  PARSE_OPT_NOARG | PARSE_OPT_NONEG, (f) }

/**
 * Introduce an option with a filename argument.
 * The filename will be prefixed by passing the filename along with
 * the prefix argument of `parse_options()` to `prefix_filename()`.
 */
#define OPT_FILENAME(s, l, v, h)    { OPTION_FILENAME, (s), (l), (v), \
				       N_("file"), (h) }

/**
 * Introduce an option that takes an optional argument that can
 * have one of three values: "always", "never", or "auto".  If the
 * argument is not given, it defaults to "always".  The `--no-` form
 * works like `--long=never`; it cannot take an argument.  If
 * "always", set `int_var` to 1; if "never", set `int_var` to 0; if
 * "auto", set `int_var` to 1 if stdout is a tty or a pager,
 * 0 otherwise.
 */
#define OPT_COLOR_FLAG(s, l, v, h) \
	{ OPTION_CALLBACK, (s), (l), (v), N_("when"), (h), PARSE_OPT_OPTARG, \
		parse_opt_color_flag_cb, (intptr_t)"always" }

/**
 * Introduce an option that has no effect and takes no arguments.
 * Use it to hide deprecated options that are still to be recognized
 * and ignored silently.
 */
#define OPT_NOOP_NOARG(s, l) \
	{ OPTION_CALLBACK, (s), (l), NULL, NULL, \
	  N_("no-op (backward compatibility)"),		\
	  PARSE_OPT_HIDDEN | PARSE_OPT_NOARG, parse_opt_noop_cb }

#define OPT_ALIAS(s, l, source_long_name) \
	{ OPTION_ALIAS, (s), (l), (source_long_name) }

/*
 * parse_options() will filter out the processed options and leave the
 * non-option arguments in argv[]. argv0 is assumed program name and
 * skipped.
 *
 * usagestr strings should be marked for translation with N_().
 *
 * Returns the number of arguments left in argv[].
 *
 * In one-shot mode, argv0 is not a program name, argv[] is left
 * untouched and parse_options() returns the number of options
 * processed.
 */
int parse_options(int argc, const char **argv, const char *prefix,
		  const struct option *options,
		  const char * const usagestr[], int flags);

NORETURN void usage_with_options(const char * const *usagestr,
				 const struct option *options);

NORETURN void usage_msg_opt(const char *msg,
			    const char * const *usagestr,
			    const struct option *options);

int optbug(const struct option *opt, const char *reason);
const char *optname(const struct option *opt, int flags);

/*
 * Use these assertions for callbacks that expect to be called with NONEG and
 * NOARG respectively, and do not otherwise handle the "unset" and "arg"
 * parameters.
 */
#define BUG_ON_OPT_NEG(unset) do { \
	if ((unset)) \
		BUG("option callback does not expect negation"); \
} while (0)
#define BUG_ON_OPT_ARG(arg) do { \
	if ((arg)) \
		BUG("option callback does not expect an argument"); \
} while (0)

/*
 * Similar to the assertions above, but checks that "arg" is always non-NULL.
 * This assertion also implies BUG_ON_OPT_NEG(), letting you declare both
 * assertions in a single line.
 */
#define BUG_ON_OPT_NEG_NOARG(unset, arg) do { \
	BUG_ON_OPT_NEG(unset); \
	if(!(arg)) \
		BUG("option callback expects an argument"); \
} while(0)

/*----- incremental advanced APIs -----*/

/*
 * It's okay for the caller to consume argv/argc in the usual way.
 * Other fields of that structure are private to parse-options and should not
 * be modified in any way.
 */
struct parse_opt_ctx_t {
	const char **argv;
	const char **out;
	int argc, cpidx, total;
	const char *opt;
	int flags;
	const char *prefix;
	const char **alias_groups; /* must be in groups of 3 elements! */
	struct option *updated_options;
};

void parse_options_start(struct parse_opt_ctx_t *ctx,
			 int argc, const char **argv, const char *prefix,
			 const struct option *options, int flags);

int parse_options_step(struct parse_opt_ctx_t *ctx,
		       const struct option *options,
		       const char * const usagestr[]);

int parse_options_end(struct parse_opt_ctx_t *ctx);

struct option *parse_options_dup(const struct option *a);
struct option *parse_options_concat(struct option *a, struct option *b);

/*----- some often used options -----*/
int parse_opt_abbrev_cb(const struct option *, const char *, int);
int parse_opt_expiry_date_cb(const struct option *, const char *, int);
int parse_opt_color_flag_cb(const struct option *, const char *, int);
int parse_opt_verbosity_cb(const struct option *, const char *, int);
/* value is struct oid_array* */
int parse_opt_object_name(const struct option *, const char *, int);
/* value is struct object_id* */
int parse_opt_object_id(const struct option *, const char *, int);
int parse_opt_commits(const struct option *, const char *, int);
int parse_opt_commit(const struct option *, const char *, int);
int parse_opt_tertiary(const struct option *, const char *, int);
int parse_opt_string_list(const struct option *, const char *, int);
int parse_opt_noop_cb(const struct option *, const char *, int);
enum parse_opt_result parse_opt_unknown_cb(struct parse_opt_ctx_t *ctx,
					   const struct option *,
					   const char *, int);

/**
 * Introduce an option that will be reconstructed into a char* string,
 * which must be initialized to NULL. This is useful when you need to
 * pass the command-line option to another command. Any previous value
 * will be overwritten, so this should only be used for options where
 * the last one specified on the command line wins.
 */
int parse_opt_passthru(const struct option *, const char *, int);

/**
 * Introduce an option where all instances of it on the command-line will
 * be reconstructed into an argv_array. This is useful when you need to
 * pass the command-line option, which can be specified multiple times,
 * to another command.
 */
int parse_opt_passthru_argv(const struct option *, const char *, int);

/* Add `-v, --verbose`. */
#define OPT__VERBOSE(var, h)  OPT_COUNTUP('v', "verbose", (var), (h))

/* Add `-q, --quiet`. */
#define OPT__QUIET(var, h)    OPT_COUNTUP('q', "quiet",   (var), (h))

#define OPT__VERBOSITY(var) \
	{ OPTION_CALLBACK, 'v', "verbose", (var), NULL, N_("be more verbose"), \
	  PARSE_OPT_NOARG, &parse_opt_verbosity_cb, 0 }, \
	{ OPTION_CALLBACK, 'q', "quiet", (var), NULL, N_("be more quiet"), \
	  PARSE_OPT_NOARG, &parse_opt_verbosity_cb, 0 }

/* Add `-n, --dry-run`. */
#define OPT__DRY_RUN(var, h)  OPT_BOOL('n', "dry-run", (var), (h))

/* Add `-f, --force`. */
#define OPT__FORCE(var, h, f) OPT_COUNTUP_F('f', "force",   (var), (h), (f))

/* Add `--abbrev[=<n>]`. */
#define OPT__ABBREV(var)  \
	{ OPTION_CALLBACK, 0, "abbrev", (var), N_("n"),	\
	  N_("use <n> digits to display SHA-1s"),	\
	  PARSE_OPT_OPTARG, &parse_opt_abbrev_cb, 0 }

/* Add `--color[=<when>]` and `--no-color`. */
#define OPT__COLOR(var, h) \
	OPT_COLOR_FLAG(0, "color", (var), (h))

#define OPT_COLUMN(s, l, v, h) \
	{ OPTION_CALLBACK, (s), (l), (v), N_("style"), (h), PARSE_OPT_OPTARG, parseopt_column_callback }
#define OPT_PASSTHRU(s, l, v, a, h, f) \
	{ OPTION_CALLBACK, (s), (l), (v), (a), (h), (f), parse_opt_passthru }
#define OPT_PASSTHRU_ARGV(s, l, v, a, h, f) \
	{ OPTION_CALLBACK, (s), (l), (v), (a), (h), (f), parse_opt_passthru_argv }
#define _OPT_CONTAINS_OR_WITH(name, variable, help, flag) \
	{ OPTION_CALLBACK, 0, name, (variable), N_("commit"), (help), \
	  PARSE_OPT_LASTARG_DEFAULT | flag, \
	  parse_opt_commits, (intptr_t) "HEAD" \
	}
#define OPT_CONTAINS(v, h) _OPT_CONTAINS_OR_WITH("contains", v, h, PARSE_OPT_NONEG)
#define OPT_NO_CONTAINS(v, h) _OPT_CONTAINS_OR_WITH("no-contains", v, h, PARSE_OPT_NONEG)
#define OPT_WITH(v, h) _OPT_CONTAINS_OR_WITH("with", v, h, PARSE_OPT_HIDDEN | PARSE_OPT_NONEG)
#define OPT_WITHOUT(v, h) _OPT_CONTAINS_OR_WITH("without", v, h, PARSE_OPT_HIDDEN | PARSE_OPT_NONEG)
#define OPT_CLEANUP(v) OPT_STRING(0, "cleanup", v, N_("mode"), N_("how to strip spaces and #comments from message"))
#define OPT_PATHSPEC_FROM_FILE(v) OPT_FILENAME(0, "pathspec-from-file", v, N_("read pathspec from file"))
#define OPT_PATHSPEC_FILE_NUL(v)  OPT_BOOL(0, "pathspec-file-nul", v, N_("with --pathspec-from-file, pathspec elements are separated with NUL character"))

#endif
