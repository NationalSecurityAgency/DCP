
bin_PROGRAMS=dcp
dcp_SOURCES=main.c digest.c cmdline.c io/io_entry.c io/io_metadata.c          \
    io/pack.c io/io_index.c io/io_xattr.c index/db_index.c io_dcp_processor.c \
    logging.c fd.c impl/dcp.c impl/process_regular.c impl/process_directory.c \
    impl/process_symlink.c impl/preprocess.c impl/process_special.c
dcp_CPPFLAGS=-Wall -Wextra -Werror -fpie -Wno-unused-but-set-variable
dcp_LDFLAGS=-lcrypto -ljansson -ldb -pie

# ensure the headers make it into the dist tarball
EXTRA_DIST=digest.h cmdline.h io/io_entry.h io/io_metadata.h io/pack.h        \
    io/io.h io/io_index.h io/io_xattr.h fd.h index/index.h io_dcp_processor.h \
    logging.h entry.h impl/dcp.h impl/process.h
    
