
#include "pathutils.h"

#if defined(_WIN32) && !defined(_CYGWIN)

// custom impl. of POSIX's dirname
// (note that it modifies buffer pointed by path)
char* dirname(char* path)
{
    if (!path)
        return NULL;

    int len = strlen(path);
    if (len < 1)
        return NULL;

    char* last_slash = strrchr(path, '/');
    if (!last_slash) {
        // no slash - no directory in path (only basename)
        path[0] = '.';
        path[1] = '\0';
        return path;
    }

    if (path + len - 1 == last_slash) {
        // slash at the end of the path - remove it
        path[len - 1] = '\0';
    }

    last_slash = strrchr(path, '/');
    if (!last_slash) {
        // no directory before basename
        path[0] = '.';
        path[1] = '\0';
        return path;
    }
    else {
        // cut the last slash and the basename from path
        *last_slash = '\0';
        return path;
    }
}

// custom impl. of POSIX's basename
// (note that it modifies buffer pointed by path)
char* basename(char* path)
{
    if (!path)
        return NULL;

    char* last_slash = strrchr(path, '/');
    if (!last_slash) {
        // no slash - no directory in path (only basename)
        return path;
    }

    int len = strlen(path);
    if (path + len - 1 == last_slash) {
        // slash at the end of the path - remove it
        path[len - 1] = '\0';
    }

    last_slash = strrchr(path, '/');
    if (!last_slash) {
        // no directory before basename
        return path;
    }
    else {
        // the basename starts after the last slash
        return last_slash + 1;
    }
}
#endif
