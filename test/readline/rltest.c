#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/errno.h>

#include <readline/readline.h>
#include <readline/history.h>

extern char *xmalloc ();

/* The names of functions that actually do the manipulation. */
int com_list __P((char *));
int com_view __P((char *));
int com_rename __P((char *));
int com_stat __P((char *));
int com_pwd __P((char *));
int com_delete __P((char *));
int com_help __P((char *));
int com_cd __P((char *));
int com_quit __P((char *));

typedef struct {
  char *name;			/* User printable name of the function. */
  rl_icpfunc_t *func;		/* Function to call to do the job. */
  char *doc;			/* Documentation for this function.  */
} COMMAND;

COMMAND commands[] = {
  { "cd", com_cd, "Change to directory DIR" },
  { "delete", com_delete, "Delete FILE" },
  { "help", com_help, "Display this text" },
  { "?", com_help, "Synonym for `help'" },
  { "list", com_list, "List files in DIR" },
  { "ls", com_list, "Synonym for `list'" },
  { "pwd", com_pwd, "Print the current working directory" },
  { "quit", com_quit, "Quit using Fileman" },
  { "rename", com_rename, "Rename FILE to NEWNAME" },
  { "stat", com_stat, "Print out statistics on FILE" },
  { "view", com_view, "View the contents of FILE" },
  { (char *)NULL, (rl_icpfunc_t *)NULL, (char *)NULL }
};

char *stripwhite ();
COMMAND *find_command ();

/* The name of this program, as taken from argv[0]. */
char *progname;

/* When non-zero, this means the user is done using this program. */
int done;

char *dupstr (int s) {
    char *r;

    r = xmalloc (strlen (s) + 1);
    strcpy (r, s);
    return (r);
}

main (int argc, char **argv) {
    char *line, *s;

    progname = argv[0];
    initialize_readline ();	/* Bind our completer. */
    while (done == 0) {
        line = readline ("FileMan: ");

        if (!line)
            break;

        s = stripwhite (line);
        if (*s) {
            add_history (s);
            execute_line (s);
        }
        free (line);
    }
    exit (0);
}

/* Execute a command line. */
int execute_line (char *line) {
    register int i;
    COMMAND *command;
    char *word;

    /* Isolate the command word. */
    for (i = 0; line[i] && whitespace (line[i]); i++)
        ;
    for (word = line + i; line[i] && !whitespace (line[i]); i++)
        ;
    if (line[i])
        line[i++] = '\0';
    command = find_command (word);
    if (!command) {
        fprintf (stderr, "%s: No such command for FileMan.\n", word);
        return (-1);
    }

    /* Get argument to command, if any. */
    for (;whitespace (line[i]); i++)
        ;
    word = line + i;

    /* Call the function. */
    return ((*(command->func)) (word));
}

/* Look up NAME as the name of a command, and return a pointer to that
   command.  Return a NULL pointer if NAME isn't a command name. */
COMMAND *find_command (char *name) {
    int i;

    for (i = 0; commands[i].name; i++)
        if (strcmp (name, commands[i].name) == 0)
            return (&commands[i]);
    return ((COMMAND *)NULL);
}

/* Strip whitespace from the start and end of STRING.  Return a pointer
   into STRING. */
char *stripwhite (char *string) {
    char *s, *t;

    for (s = string; whitespace (*s); s++)
        ;
    if (*s == 0)
        return (s);

    t = s + strlen (s) - 1;
    while (t > s && whitespace (*t))
        t--;
    *++t = '\0';

    return s;
}

char *command_generator __P((const char *, int));
char **fileman_completion __P((const char *, int, int));

/* Tell the GNU Readline library how to complete.  We want to try to
   complete on command names if this is the first word in the line, or
   on filenames if not. */
initialize_readline () {
    /* Allow conditional parsing of the ~/.inputrc file. */
    rl_readline_name = "FileMan";

    /* Tell the completer that we want a crack first. */
    rl_attempted_completion_function = fileman_completion;
}

/* Attempt to complete on the contents of TEXT.  START and END
   bound the region of rl_line_buffer that contains the word to
   complete.  TEXT is the word to complete.  We can use the entire
   contents of rl_line_buffer in case we want to do some simple
   parsing.  Returnthe array of matches, or NULL if there aren't any. */
char **fileman_completion ( const char *text, int start, int end) {
    char **matches;

    matches = (char **)NULL;

    /* If this word is at the start of the line, then it is a command
        to complete.  Otherwise it is the name of a file in the current
        directory. */
    if (start == 0)
        matches = rl_completion_matches (text, command_generator);

    return (matches);
}

/* Generator function for command completion.  STATE lets us
   know whether to start from scratch; without any state
   (i.e. STATE == 0), then we start at the top of the list. */
char *command_generator (const char *text, int state) {
    static int list_index, len;
    char *name;

    /* If this is a new word to complete, initialize now.  This
        includes saving the length of TEXT for efficiency, and
        initializing the index variable to 0. */
    if (!state) {
        list_index = 0;
        len = strlen (text);
    }

    /* Return the next name which partially matches from the
        command list. */
    while (name = commands[list_index].name) {
        list_index++;

        if (strncmp (name, text, len) == 0)
            return (dupstr(name));
    }

    /* If no names matched, then return NULL. */
    return ((char *)NULL);
}

/* String to pass to system ().  This is for the LIST, VIEW and RENAME
   commands. */
static char syscom[1024];

/* List the file(s) named in arg. */
com_list (char *arg) {
    if (!arg)
        arg = "";

    sprintf (syscom, "ls -FClg %s", arg);
    return (system (syscom));
}

com_view (char *arg) {
    if (!valid_argument ("view", arg))
        return 1;

    sprintf (syscom, "more %s", arg);
    return (system (syscom));
}

com_rename (char *arg) {
    too_dangerous ("rename");
    return (1);
}

com_stat (char *arg) {
    struct stat finfo;

    if (!valid_argument ("stat", arg))
        return (1);

    if (stat (arg, &finfo) == -1) {
        perror (arg);
        return (1);
    }
    printf ("Statistics for `%s':\n", arg);
    printf ("%s has %d link%s, and is %d byte%s in length.\n", arg,
            finfo.st_nlink,
            (finfo.st_nlink == 1) ? "" : "s",
            finfo.st_size,
            (finfo.st_size == 1) ? "" : "s");
    printf ("Inode Last Change at: %s", ctime (&finfo.st_ctime));
    printf ("      Last access at: %s", ctime (&finfo.st_atime));
    printf ("    Last modified at: %s", ctime (&finfo.st_mtime));
    return (0);
}

com_delete (char *arg) {
    too_dangerous ("delete");
    return (1);
}

/* Print out help for ARG, or for all of the commands if ARG is
   not present. */
com_help (char *arg) {
    int i;
    int printed = 0;

    for (i = 0; commands[i].name; i++) {
        if (!*arg || (strcmp (arg, commands[i].name) == 0)) {
            printf ("%s\t\t%s.\n", commands[i].name, commands[i].doc);
            printed++;
        }
    }

    if (!printed) {
        printf ("No commands match `%s'.  Possibilties are:\n", arg);
        for (i = 0; commands[i].name; i++) {
            /* Print in six columns. */
            if (printed == 6) {
                printed = 0;
                printf ("\n");
            }

            printf ("%s\t", commands[i].name);
            printed++;
        }

        if (printed)
            printf ("\n");
    }
    return (0);
}

/* Change to the directory ARG. */
com_cd (char *arg) {
    if (chdir (arg) == -1) {
        perror (arg);
        return 1;
    }

    com_pwd ("");
    return (0);
}

/* Print out the current working directory. */
com_pwd (char *ignore) {
    char dir[1024], *s;

    s = getcwd (dir, sizeof(dir) - 1);
    if (s == 0) {
        printf ("Error getting pwd: %s\n", dir);
        return 1;
    }

    printf ("Current directory is %s\n", dir);
    return 0;
}

/* The user wishes to quit using this program.  Just set DONE
   non-zero. */
com_quit (char *arg) {
    done = 1;
    return (0);
}

/* Function which tells you that you can't do this. */
too_dangerous (char *caller) {
    fprintf (stderr,
            "%s: Too dangerous for me to distribute.\n",
            caller);
    fprintf (stderr, "Write it yourself.\n");
}

/* Return non-zero if ARG is a valid argument for CALLER,
   else print an error message and return zero. */
int valid_argument (char *caller, char *arg) {
    if (!arg || !*arg) {
        fprintf (stderr, "%s: Argument required.\n", caller);
        return (0);
    }

    return (1);
}
