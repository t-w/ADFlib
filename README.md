```
      _/_/    _/_/_/    _/_/_/_/  _/  _/  _/
   _/    _/  _/    _/  _/        _/      _/_/_/
  _/_/_/_/  _/    _/  _/_/_/    _/  _/  _/    _/
 _/    _/  _/    _/  _/        _/  _/  _/    _/
_/    _/  _/_/_/    _/        _/  _/  _/_/_/

Amiga Disk File library
```

The `ADFlib` is a free, portable and open implementation of the Amiga filesystem.

The library is written in portable C (currently C99).

The origins of the library reaches late 1990s, when the initial version
was designed and developed by Laurent Clévy on his Amiga 1200 (with
a big-endian CPU: MC68EC020 14MHz).

Recent versions were built and tested in a CI system (GitHub Actions)
on Linux (Debian, Ubuntu), Windows (MSVC, CygWin and MSYS2) and MacOs.

It should be possible to build on (or cross-compile for) other systems.


## Features
The main purpose of the library is to allow read and write Amiga-formatted
devices and files being byte-level copies of such devices, called disk images
or dumps. In case of classic Amiga systems, such files are most often
[ADFs](https://en.wikipedia.org/wiki/Amiga_Disk_File) or HDFs ("Hard Disk
Files") contaning
[OFS](https://en.wikipedia.org/wiki/Amiga_Old_File_System) or
[FFS](https://en.wikipedia.org/wiki/Amiga_Fast_File_System) (there are also
other Amiga filesystems, for instance
[PFS](https://en.wikipedia.org/wiki/Professional_File_System)).

ADFlib allows accesing the aforementioned devices on 3 levels:
1. device level - access to blocks (storage units) of the whole device
2. volume level - access to blocks of the logical parts of the device,
   which, in case of the Amiga ecosystem, are called volumes
3. filesystem level - access to files and directories stored inside volumes.

The library supports:
- ADF/HDF devices - floppy disks (DD/HD) and hard disks (up to 2GB)
  - create devices and volumes and read/write blocks
  - device types: files, physical ("native") devices and ramdisk
    - possibility to implement other types via "drivers"
- filesystem:
  - "classic" Amiga filesystems: OFS and FFS
  - file operations: read, write, truncate
  - directory operations: get contents, create, remove, rename, move files
    and directories
  - use dir cache to get directory contents
  - use hard- and softlinks for accessing files and directories
  - rebuild block allocation bitmap

Untested and/or experimental support exists for:
- WinNT and Linux physical devices (with the 'native driver')
- multiple partitions harddisk dumps
- UAE hardfiles
- undel files and directories


## Command-line utilities
The `examples/` directory contains few useful command-line utilities
(as well as examples of the library usage).

Usage info is shown when they are executed without any parameters
(see also man pages).


### unADF

`unADF` is an unzip-like utility for `.ADF` files.

```
Usage:  unadf [-lrcsmpwV] [-v n] [-d extractdir] dumpname.adf [files-with-path]

    -l : lists root directory contents
    -r : lists directory tree contents
    -c : use dircache data (must be used with -l)
    -s : display entries logical block pointer (must be used with -l)
    -m : display file comments, if exists (must be used with -l)
    -p : send extracted files to pipe (unadf -p dump.adf Pics/pic1.gif | xv -)
    -w : mangle filenames to be compatible with Windows filesystems
    -h : show help
    -V : show version

    -v n : mount volume #n instead of default #0 volume
    -d dir : extract to 'dir' directory
```

### adfimgcreate
Create empty (not formatted!) ADF/HDF image files of specified type or size.

### adf_format

Create a classic AmigaDOS filesystem (OFS or FFS) on the specified volume
of an ADF/HDF device.

Current limitation: it supports only unpartitioned devices/disk images,
so ones without an RDSK block and the typical hard disk structure.
Only devices with a single volume on the whole device, so only floppy disks
(ADF) or unpartitioned hard disk file (HDF) devices can be formatted.


### adf_show_metadata

A low-level utility / diagnostic tool, showing metadata about a device / device
image or a file/directory inside the Amiga filesystem. In particular, it shows
contents of Amiga filesystem metadata blocks, it can help understand structure
of Amiga filesystems (for anyone curious...).

### adf_bitmap

A low-level utility / diagnostic tool for block allocation bitmap of ADF volumes.
It can display the bitmap or rebuild it (in fact, enforce rebuilding it, even if
the volume's flag says that the bitmap is valid).

### adf_salvage

An utility allowing to list deleted entries (files, directories) on a volume
and undelete them (in the future possibly also extract them to local filesystem).

## Credits:

- main design and code : Laurent Clévy
- current maintainer, recent core developments (Dec 2022-): Tomasz Wolak
- unadf rewrite (in 2019) : Stuart Caie
- Bug fixes and C++ wrapper : Bjarke Viksoe (adfwrapper.h)
- WinNT native driver : Dan Sutherland and Gary Harris
- See also: AUTHORS

## Bugs
See BUGS.md

## Compilation and installation

See INSTALL file.


## Files
- `AUTHORS` : Contributors
- `README.md` : The file you are reading
- `ChangeLog` : updates in subsequent versions
- `INSTALL` : compilation, testing and installation information
- `TODO` : possible future changes/improvements
- `src/` :	main library files
- `src/win32/` : WinNT native driver (untested!)
- `src/linux/` : Linux native driver (experimental!)
- `src/generic/` : native files templates ("dummy" device)
- `regtests/Boot/` :	Bootblocks that might by used to put on floppy disks
- `doc/` :	The library developer's documentation, man pages for utilities
- `doc/FAQ/` : The Amiga Filesystem explained
- `examples/` : utilities: `unadf`, `adfimgcreate`, `adf_format`,
`adf_show_metadata`, `adf_bitmap`, `adf_salvage`
- `packaging` : misc. packaging configurations (so far - deb only)
- `regtests/` : regression tests
- `tests/` : unit tests


## Current status
Most of the code has certain age (the lib was designed and in large part
implemented before the year 2000). Some subsystems have improved, but still
some remain untouched for a long time and may not have dedicated tests
(meaning: are not tested).

### Native devices
The library is mostly tested with ADF disk (ie. floppy) images, not with
any native/real disk devices. Since version 0.9.0, the native devices
are not enabled by default, so it is now more safe to keep them compiled.
However, if they are not needed at all, the library still can be build
without them (with the `generic` (dummy) native device instead, see
INSTALL for details).

Enabling and using the devices remember that **a native device can be
any physical storage device on your system(!)**. Please _know what you
are doing_ (eg. do not open your `C:\` on windows or `/dev/sda` on Linux
and call a function for creating an Amiga filesystem on it... unless
you really want to reinstall your system and restore your data
from a backup...).

Since native devices are not much tested - consider them as
testing/experimental and treat as such (ie. use in a safe environment,
like a VM).

### Write support
The file read and write support are rather well tested (except dir. cache,
see below!), but still, writing is a new and potentially harmul feature
so do not experiment on a unique copy of an ADF image with your precious
data. Do it on a copy (and report if any issues are encountered).

Update: The notice above is especially actual as it was discovered that
the version `0.8.0` and earlier **do not rebuild the block allocation bitmap
for volumes where it is marked invalid**. In short, this means that if the bitmap
is really incorrect, writing to such volume may lead to data loss/corruption
(blocks used by stored data might not be marked as such!).
Because of this, it is strongly advised to **UPDATE TO THE LATEST VERSION**.

(See also TODO and BUGS).

### DirCache
Dircache is an existing but somewhat exotic feature of the FFS.
Until now, I haven't encountered any disk image with this enabled. The only
one with dircache set on the volume is one of the test floppies for the ADFlib
(testffs.adf)...

While dircache support is implemented in ADFlib (at least, to certain extent),
so far, there are no tests of dircache. Assume that, as such, this feature is
practically **not tested**.
While it can be used rather safely in read-only mode, be very careful with
write mode using a volume with dircache enabled.


## Documentation

- ADF format (devices and filesystems):
  - [The .ADF (Amiga Disk File) format FAQ](http://lclevy.free.fr/adflib/adf_info.html)
    by [Laurent Clévy](http://lclevy.free.fr/) (also
	[here](https://adflib.github.io/FAQ/adf_info.html)) - a comprehensive study
	of the Amiga storage technology - from disk data encoding through disk
	geometry to	filesystem structure (OFS/FFS)
  - [Wikipedia article](https://en.wikipedia.org/wiki/Amiga_Disk_File)

### The API documentation in doc/
... in major part is outdated. The lib underwent many changes and, possibly,
some still to come. Not enough time to fully deal with this, too...

Please use it as a reference for the concepts and general ways of use (as those
have not changed) while for the details regarding functions check the header
files, sources in `tests/` and `examples/` to see in details how the current
API can be used.

<!--Please report any bugs or mistakes in the documentation !-->

Have fun anyway !


## Contributing
If you encountered a problem, please review
[the existing issues](https://github.com/adflib/ADFlib/issues) and,
if the problem you have is not already there, open a new one.

For bugfixes and/or new things - please open a _Pull Request_ to the `devel`
branch (not the `master`).

### Repository structure
Using the code from the repository will be easier understanding that the project
has adapted
[a new branching model](https://nvie.com/posts/a-successful-git-branching-model/)
and so far use the following:
- the `master` branch contains the latest tagged and released code
- the `devel` branch contains development code that was tested and accepted
  for future releases
- other existing branches are new "feature" branches (that may or may not be merged)

Other things of the scheme may be used/adapted as needed (hotfixes for released code
and such, very likely release branch(es) will also appear).


## Related projects

### Projects using ADFlib
- [adfExplorer in R](https://github.com/pepijn-devries/adfExplorer)
- [DiskFlashBack](https://robsmithdev.co.uk/diskflashback) ([GitHub](https://github.com/RobSmithDev/DiskFlashback)) - Windows app for viewing and editing misc. disk formats (ADF among them).
- [`fuseadf`](https://gitlab.com/t-m/fuseadf) - FUSE-based Linux filesystem allowing
to mount and access ADF images in read/write mode.
- [`patool`](https://pypi.org/project/patool/) - an archive file manager written
in Python.

#### Older projects (not sure if maintained)
- [AdfView](https://www.viksoe.dk/code/adfview.htm) - Windows shell extension
- [AdfOpus](https://adfopus.sourceforge.net/) - Windows file management tool for ADF files.
- [CopyToAdf](https://www.pouet.net/prod_nfo.php?which=65625&font=4) - part of WinUAE Demo Toolchain 5
- [pyadf](https://sourceforge.net/projects/pyadf/) - Python bindings (for older versions)
  - note that this is neither
    [scripting framework for multiscale quantum chemistry](https://onlinelibrary.wiley.com/doi/full/10.1002/jcc.21810)
	([on GitHub](https://github.com/chjacob-tubs/pyadf-releases))
    nor [a Python library to help with Atlassian Document Format](https://github.com/tamus-cyber/pyadf)
	(what a popular name this is...)

### Other
- [adflib in Rust](https://github.com/vschwaberow/adflib)
- [affs Linux kernel module](https://git.kernel.org/pub/scm/linux/kernel/git/stable/linux.git/tree/Documentation/filesystems/affs.rst)
