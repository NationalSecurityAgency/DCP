
# DCP - Copy and Profile files and directories

The project has been refactored to use a more canonical build process. To do so, you now just need to configure and run `make`. 

```bash
./configure
make
make install
```

 > <strong>Note:</strong> You will probably need to run `make install` as root.

You will still need [gengetopt](https://www.gnu.org/software/gengetopt/gengetopt.html), as it's used to generate a command line parser for the application, but now you will be explicitly told so during the configuration process if you do not have it. If the configuration process succeeds, `gengetopt` was successfully found in your path.

Here is a breakdown of the libraries you will need to successfully build this application.

 | Library Name | Description | Version Tested |
 | ------------ | ----------- | ------- |
 | [libcrypto](https://wiki.openssl.org/index.php/Libcrypto_API)    | From OpenSSL |    -    |
 | [libdb](https://github.com/berkeleydb/libdb)        | Berkley DB   | 5.3.28-4 |
 | [gengetopt](https://www.gnu.org/software/gengetopt/gengetopt.html)    | Command line args parser generator | 2.22.6-7 |

 ## Configuration

The build system refactoring process is still ongoing, so the configuration script is not as robust as it could be. It does, however, successfully build on [gcc](https://gcc.gnu.org/), [clang](http://llvm.org/), and [Intel's](https://software.intel.com/en-us/c-compilers) `icc` compiler.

Configuration is pretty standard. You can modify the build as usual through the `CFLAGS`, `CPPFLAGS`, and `LDFLAGS` as always, just keep in mind that currently the compiler set by `CC` is used as the linker driver, so whatever settings you set in `LDFLAGS` right now must play well with whatever C compiler you choose to use. Also, any additional libraries you wish to link against should be set via the `LIBS` variable, not `LDFLAGS`.

If you run into any weird errors on a rebuild, the safest bet is to run `make distclean` then run `./configure` again to regenerate the header files for the build.

> You can not yet directly install it using the usual `make install` command, however. This is a work in progress, as the entire project had to be refactored.

You should now be able to successfully install `dcp` after building, by way of the usual `make install` command. It does not yet use `install`, but rather simply moves the newly built binary into the destination directory set during configuration. The default prefix is the usual `/usr/local` and can be configured using `--prefix=/usr`, or whatever you so wish.

You can also uninstall the binary by running `make uninstall` as root, and the binary installed by `make install` will be removed.

### Supported Compilers

This is a breakdown of the compilers on which the current version of the project has been tested. The GCC trunk branch was current as of 9:30 PM Eastern time, June 4th, 2019. All of the builds were tested on an x86-64 Arch Linux system. If you run into any issues in a different environment, be sure to include that in the issue description so we can spin up a VM with the right OS.

#### GCC

| Version | Tested | Status |
| ------- | ------ | ------ |
|  8.3.0  | Yes    | OK     |
|  9.1.0  | Yes    | OK     |
| 10.0.0  | Yes    | OK     |
| trunk[1]   | Yes    | OK     |

[1] gcc (GCC) 10.0.0 20190605 (experimental)

#### Clang

| Version | Tested | Status |
| ------- | ------ | ------ |
| 8.0.0-4 | Yes    | OK     |

#### Intel ICC

| Version | Tested | Status |
| ------- | ------ | ------ |
| 19.0.4.235 | Yes | OK     |
