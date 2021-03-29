/*
 * Copyright (C) 2003  Serge van den Boom
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 * Nota bene: later versions of the GNU General Public License do not apply
 * to this program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

typedef struct ziptree_Handle *uio_NativeHandle;
typedef void *uio_GPRootExtra;
typedef struct ziptree_GPFileData *uio_GPFileExtra;
typedef struct ziptree_GPFileData *uio_GPDirExtra;
typedef struct uio_GPDirEntries_Iterator *uio_NativeEntriesContext;

#define uio_INTERNAL_PHYSICAL

#include "../gphys.h"
#include "../iointrn.h"
#include "../uioport.h"
#include "../physical.h"
#include "../types.h"
#include "../fileblock.h"

#include <zlib.h>
#include <sys/types.h>
#include <sys/stat.h>

typedef struct ziptree_GPFileData {
	off_t compressedSize;
	off_t uncompressedSize;
	uio_uint16 compressionFlags;
	uio_uint16 compressionMethod;
#if zip_USE_HEADERS == zip_USE_CENTRAL_HEADERS
	off_t headerOffset;  // start of the local header for this file
#endif
	off_t fileOffset;  // start of the compressed data in the .zip file
	uid_t uid;
	gid_t gid;
	mode_t mode;
	time_t atime;  // access time
	time_t mtime;  // modification time
	time_t ctime;  // change time
} ziptree_GPFileData;

typedef ziptree_GPFileData ziptree_GPDirData;

typedef struct ziptree_Handle {
	uio_GPFile *file;
	z_stream zipStream;
	uio_FileBlock *fileBlock;
	off_t uncompressedOffset;
			// seek location in the uncompressed stream
	off_t compressedOffset;
			// seek location in the compressed stream, from the start
			// of the compressed file
} ziptree_Handle;


uio_PRoot *ziptree_mount(uio_Handle *handle, int flags);
int zip_umount(struct uio_PRoot *);
uio_Handle *ziptree_open(uio_PDirHandle *pDirHandle, const char *file, int flags,
		mode_t mode);
void ziptree_close(uio_Handle *handle);
int ziptree_access(uio_PDirHandle *pDirHandle, const char *name, int mode);
int ziptree_fstat(uio_Handle *handle, struct stat *statBuf);
int ziptree_stat(uio_PDirHandle *pDirHandle, const char *name,
		struct stat *statBuf);
ssize_t ziptree_read(uio_Handle *handle, void *buf, size_t count);
off_t ziptree_seek(uio_Handle *handle, off_t offset, int whence);
