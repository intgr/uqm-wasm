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

/*
 * This file makes use of zlib (http://www.gzip.org/zlib/)
 *
 * References:
 * The .zip format description from PKWare:
 *   http://www.pkware.com/products/enterprise/white_papers/appnote.html
 * The .zip format description from InfoZip:
 *   ftp://ftp.info-zip.org/pub/infozip/doc/appnote-011203-iz.zip
 */
#define _GNU_SOURCE

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>

#include "ziptree.h"
#include "../physical.h"
#include "../uioport.h"
#include "../paths.h"
#include "../uioutils.h"
#ifdef uio_MEM_DEBUG
#	include "../memdebug.h"
#endif

// FIXME cleanup!
#undef DEBUG
#define DEBUG 2

static inline ziptree_GPFileData * ziptree_GPFileData_new(void);
static inline void ziptree_GPFileData_delete(ziptree_GPFileData *gPFileData);
static inline ziptree_GPFileData *ziptree_GPFileData_alloc(void);
static inline void ziptree_GPFileData_free(ziptree_GPFileData *gPFileData);
static inline void ziptree_GPDirData_delete(ziptree_GPDirData *gPDirData);
static inline void ziptree_GPDirData_free(ziptree_GPDirData *gPDirData);

extern uio_Repository *repository;
extern uio_DirHandle *contentDir;


uio_GPDirEntries_Iterator *
ziptree_GPDir_openEntries(uio_PDirHandle *pDirHandle) {
#if defined(DEBUG) && DEBUG > 1
	fprintf(stderr, "ziptree_GPDir_openEntries - handle=%p\n", (void *) pDirHandle);
#endif
	return uio_GPDir_openEntries(pDirHandle);
}

// The ref counter for the dir entry is not incremented.
uio_PDirEntryHandle *
ziptree_GPDir_getPDirEntryHandle(const uio_PDirHandle *pDirHandle,
		const char *name) {
	char *zipPath;
#if defined(DEBUG) && DEBUG > 1
	fprintf(stderr, "ziptree_GPDir_getPDirEntryHandle - handle=%p, name=%s\n", (void *) pDirHandle, name);
#endif
/*
	contentMountHandle = uio_mountDir (repository, "/",
			uio_FSTYPE_ZIPTREE, NULL, NULL, "/", autoMount,
			uio_MOUNT_TOP | uio_MOUNT_RDONLY, NULL);
	uio_mountDir (repository, mountPoint, uio_FSTYPE_ZIP,
			dirHandle, dirList->names[i], "/", autoMount,
			relativeFlags | uio_MOUNT_RDONLY,
			relativeHandle) == NULL)
*/
	static uio_AutoMount *autoMount[] = { NULL };

	char *basedir;
	char cwd[256];
	getcwd(cwd, 256);
//	asprintf(&zipPath, "%s/ziptree/%s.zip", cwd, name);
//	asprintf(&zipPath, "ziptree/%s.zip", name);
	asprintf(&zipPath, "ziptree/%s.zip", name);
	if (strcmp(name, "ziptree") == 0) {
		return NULL;
	}
//	asprintf(&basedir, "%s/ziptree", cwd);
	if (! fileExists(zipPath)) {
		return NULL;
	}

	char *mountPount;

//	uio_DirHandle * ziptree = uio_openDir(repository, "/ziptree", 0);
//	uio_DirHandle * ziptree = uio_openDir(repository, basedir, 0);

	fprintf(stderr, "MOUNTING %s\n", zipPath);
	uio_MountHandle *mount = uio_mountDir (repository, name,
			uio_FSTYPE_ZIP, contentDir, zipPath, "/", autoMount,
			uio_MOUNT_TOP | uio_MOUNT_RDONLY, NULL);
	if (mount == NULL) {
		fprintf(stderr, "Error mounting %s as %s\n", name, zipPath);
		return NULL;
	}

//	return uio_GPDir_getPDirEntryHandle(mount->mountInfo->pDirHandle, "");
	return uio_getPDirEntryHandle(mount->mountInfo->pDirHandle, "");
//	uio_mountDir();
//	return uio_GPDir_getPDirEntryHandle (pDirHandle, name);
}


uio_FileSystemHandler ziptree_fileSystemHandler = {
	/* .init    = */  NULL,
	/* .unInit  = */  NULL,
	/* .cleanup = */  NULL,

	/* .mount  = */  ziptree_mount,
	/* .umount = */  uio_GPRoot_umount,

	/* .access = */  ziptree_access,
	/* .close  = */  ziptree_close,
	/* .fstat  = */  ziptree_fstat,
	/* .stat   = */  ziptree_stat,
	/* .mkdir  = */  NULL,
	/* .open   = */  ziptree_open,
	/* .read   = */  ziptree_read,
	/* .rename = */  NULL,
	/* .rmdir  = */  NULL,
	/* .seek   = */  ziptree_seek,
	/* .write  = */  NULL,
	/* .unlink = */  NULL,

	/* .openEntries  = */  ziptree_GPDir_openEntries,
	/* .readEntries  = */  uio_GPDir_readEntries,
	/* .closeEntries = */  uio_GPDir_closeEntries,

	/* .getPDirEntryHandle     = */  ziptree_GPDir_getPDirEntryHandle,
	/* .deletePRootExtra       = */  uio_GPRoot_delete,
	/* .deletePDirHandleExtra  = */  uio_GPDirHandle_delete,
	/* .deletePFileHandleExtra = */  uio_GPFileHandle_delete,
};

uio_GPRoot_Operations ziptree_GPRootOperations = {
	/* .fillGPDir         = */  NULL,
	/* .deleteGPRootExtra = */  NULL,
	/* .deleteGPDirExtra  = */  ziptree_GPDirData_delete,
	/* .deleteGPFileExtra = */  ziptree_GPFileData_delete,
};


void
ziptree_close(uio_Handle *handle) {
	ziptree_Handle *ziptree_handle;

#if defined(DEBUG) && DEBUG > 1
	fprintf(stderr, "ziptree_close - handle=%p\n", (void *) handle);
#endif
	ziptree_handle = handle->native;
	uio_GPFile_unref(ziptree_handle->file);
//	zip_unInitZipStream(&ziptree_handle->zipStream);
	uio_closeFileBlock(ziptree_handle->fileBlock);
	uio_free(ziptree_handle);
}

static void
zip_fillStat(struct stat *statBuf, const ziptree_GPFileData *gPFileData) {
	memset(statBuf, '\0', sizeof (struct stat));
	statBuf->st_size = gPFileData->uncompressedSize;
	statBuf->st_uid = gPFileData->uid;
	statBuf->st_gid = gPFileData->gid;
	statBuf->st_mode = gPFileData->mode;
	statBuf->st_atime = gPFileData->atime;
	statBuf->st_mtime = gPFileData->mtime;
	statBuf->st_ctime = gPFileData->ctime;
}

int
ziptree_access(uio_PDirHandle *pDirHandle, const char *name, int mode) {
	errno = ENOSYS;  // Not implemented.
	(void) pDirHandle;
	(void) name;
	(void) mode;
	return -1;
}

int
ziptree_fstat(uio_Handle *handle, struct stat *statBuf) {
#if 0
	if (handle->native->file->extra->fileOffset == -1) {
		// The local header wasn't read in yet.
		if (zip_updateFileDataFromLocalHeader(handle->root->handle,
				handle->native->file->extra) == -1) {
			// errno is set
			return -1;
		}
	}
#endif  /* zip_USE_HEADERS == zip_USE_CENTRAL_HEADERS */
	zip_fillStat(statBuf, handle->native->file->extra);
	return 0;
}

int
ziptree_stat(uio_PDirHandle *pDirHandle, const char *name, struct stat *statBuf) {
	return -1;
#if 0
	uio_GPDirEntry *entry;

	if (name[0] == '.' && name[1] == '\0') {
		entry = (uio_GPDirEntry *) pDirHandle->extra;
	} else {
		entry = uio_GPDir_getGPDirEntry(pDirHandle->extra, name);
		if (entry == NULL) {
			errno = ENOENT;
			return -1;
		}
	}
	
	if (uio_GPDirEntry_isDir(entry) && entry->extra == NULL) {
		// No information about this directory was stored.
		// We'll have to make something up.
		memset(statBuf, '\0', sizeof (struct stat));
		statBuf->st_mode = S_IFDIR | S_IRUSR | S_IWUSR | S_IXUSR |
				S_IRGRP | S_IWGRP | S_IXOTH |
				S_IROTH | S_IWOTH | S_IXOTH;
		statBuf->st_uid = 0;
		statBuf->st_gid = 0;
		return 0;
	}

#if 0
	if (((ziptree_GPFileData *) entry->extra)->fileOffset == -1) {
		// The local header wasn't read in yet.
		if (zip_updateFileDataFromLocalHeader(pDirHandle->pRoot->handle,
				(ziptree_GPFileData *) entry->extra) == -1) {
			// errno is set
			return -1;
		}
	}
#endif  /* zip_USE_HEADERS == zip_USE_CENTRAL_HEADERS */

	zip_fillStat(statBuf, (ziptree_GPFileData *) entry->extra);
	return 0;
#endif
}

/*
 * Function name: ziptree_open
 * Description:   open a file in ziptree mount -- lazy load the zip file if needed
 * Arguments:     pDirHandle - handle to the dir where to open the file
 *                name - the name of the file to open
 *                flags - flags, as to stdio open()
 *                mode - mode, as to stdio open()
 * Returns:       handle, as from stdio open()
 *                If failed, errno is set and handle is -1.
 */
uio_Handle *
ziptree_open(uio_PDirHandle *pDirHandle, const char *name, int flags,
		mode_t mode) {
	ziptree_Handle *handle;
	uio_GPFile *gPFile;

#if defined(DEBUG) && DEBUG > 1
	fprintf(stderr, "ziptree_open - pDirHandle=%p name=%s flags=%d mode=0%o\n",
			(void *) pDirHandle, name, flags, mode);
#endif
	
	if ((flags & O_ACCMODE) != O_RDONLY) {
		errno = EACCES;
		return NULL;
	}

	gPFile = (uio_GPFile *) uio_GPDir_getGPDirEntry(pDirHandle->extra, name);	
	if (gPFile == NULL) {
		errno = ENOENT;
		return NULL;
	}

#if 0
#if zip_USE_HEADERS == zip_USE_CENTRAL_HEADERS
	if (gPFile->extra->fileOffset == -1) {
		// The local header wasn't read in yet.
		if (zip_updateFileDataFromLocalHeader(pDirHandle->pRoot->handle,
				gPFile->extra) == -1) {
			// errno is set
			return NULL;
		}
	}
#endif
	
	handle = uio_malloc(sizeof (ziptree_Handle));
	uio_GPFile_ref(gPFile);
	handle->file = gPFile;
	handle->fileBlock = uio_openFileBlock2(pDirHandle->pRoot->handle,
			gPFile->extra->fileOffset, gPFile->extra->compressedSize);
	if (handle->fileBlock == NULL) {
		// errno is set
		return NULL;
	}

	if (zip_initZipStream(&handle->zipStream) == -1) {
		uio_GPFile_unref(gPFile);
		uio_closeFileBlock(handle->fileBlock);
		return NULL;
	}
	handle->compressedOffset = 0;
	handle->uncompressedOffset = 0;
	
#endif
	(void) mode;
	return uio_Handle_new(pDirHandle->pRoot, handle, flags);
}

ssize_t
ziptree_read(uio_Handle *handle, void *buf, size_t count) {
	ssize_t result;

#if defined(DEBUG) && DEBUG > 1
	fprintf(stderr, "ziptree_read - handle=%p buf=%p count=%d: ", (void *) handle,
			(void *) buf, count);
#endif
#if 0
	result = zip_readMethods[handle->native->file->extra->compressionMethod]
			(handle, buf, count);
#endif
	result = 0;
#if defined(DEBUG) && DEBUG > 1
	fprintf(stderr, "%d\n", result);
#endif
	return result;
}

off_t
ziptree_seek(uio_Handle *handle, off_t offset, int whence) {
	ziptree_Handle *zipHandle;

#if defined(DEBUG) && DEBUG > 1
	fprintf(stderr, "ziptree_seek - handle=%p offset=%d whence=%s\n",
			(void *) handle, (int) offset,
			whence == SEEK_SET ? "SEEK_SET" :
			whence == SEEK_CUR ? "SEEK_CUR" :
			whence == SEEK_END ? "SEEK_END" : "INVALID");
#endif
	zipHandle = handle->native;

#if 0
	assert(whence == SEEK_SET || whence == SEEK_CUR || whence == SEEK_END);
	switch(whence) {
		case SEEK_SET:
			break;
		case SEEK_CUR:
			offset += zipHandle->uncompressedOffset;
			break;
		case SEEK_END:
			offset += zipHandle->file->extra->uncompressedSize;
			break;
	}
	if (offset < 0) {
		offset = 0;
	} else if (offset > zipHandle->file->extra->uncompressedSize) {
		offset = zipHandle->file->extra->uncompressedSize;
	}
	return zip_seekMethods[handle->native->file->extra->compressionMethod]
			(handle, offset);
#endif
	return 0;
}

uio_PRoot *
ziptree_mount(uio_Handle *handle, int flags) {
	uio_PRoot *result;
	uio_PDirHandle *rootDirHandle;
	
	assert (handle == NULL);
	if ((flags & uio_MOUNT_RDONLY) != uio_MOUNT_RDONLY) {
		errno = EACCES;
		return NULL;
	}

//	uio_Handle_ref(handle);
	result = uio_GPRoot_makePRoot(
			uio_getFileSystemHandler(uio_FSTYPE_ZIPTREE), flags,
			&ziptree_GPRootOperations, NULL,
//			uio_GPRoot_PERSISTENT,
			0,
			handle, NULL, uio_GPDir_COMPLETE);

	rootDirHandle = uio_PRoot_getRootDirHandle(result);
#if 0
	if (zip_fillDirStructure(rootDirHandle->extra, handle) == -1) {
		int savedErrno = errno;
#ifdef DEBUG
		fprintf(stderr, "Error: failed to read the zip directory "
				"structure - %s.\n", strerror(errno));
#endif
		uio_GPRoot_umount(result);
		errno = savedErrno;
		return NULL;
	}
#endif

	return result;
}

static inline int
zip_foundFile(uio_GPDir *gPDir, const char *path, ziptree_GPFileData *gPFileData) {
	uio_GPFile *file;
	size_t pathLen;
	const char *rest;
	const char *pathEnd;
	const char *start, *end;
	char *buf;

	if (path[0] == '/')
		path++;
	pathLen = strlen(path);
	if (path[pathLen - 1] == '/') {
		fprintf(stderr, "Warning: '%s' is not a valid file name - skipped.\n",
				path);
		errno = EISDIR;
		return -1;
	}
	pathEnd = path + pathLen;

	switch (uio_walkGPPath(gPDir, path, pathLen, &gPDir, &rest)) {
		case 0:
			// The entire path was matched. The last part was not supposed
			// to be a dir.
			fprintf(stderr, "Warning: '%s' already exists as a dir - "
					"skipped.\n", path);
			errno = EISDIR;
			return -1;
		case ENOTDIR:
			fprintf(stderr, "Warning: A component to '%s' is not a "
					"directory - file skipped.\n", path);
			errno = ENOTDIR;
			return -1;
		case ENOENT:
			break;
	}

	buf = uio_malloc(pathLen + 1);
	getFirstPathComponent(rest, pathEnd, &start, &end);
	while (1) {
		uio_GPDir *newGPDir;
		
		if (end == start || (end - start == 1 && start[0] == '.') ||
				(end - start == 2 && start[0] == '.' && start[1] == '.')) {
			fprintf(stderr, "Warning: file '%s' has an invalid path - "
					"skipped.\n", path);
			uio_free(buf);
			errno = EINVAL;
			return -1;
		}
		if (end == pathEnd) {
			// This is the last component; it should be the name of the dir.
			rest = start;
			break;
		}
		memcpy(buf, start, end - start);
		buf[end - start] = '\0';
		newGPDir = uio_GPDir_prepareSubDir(gPDir, buf);
		newGPDir->flags |= uio_GPDir_COMPLETE;
				// It will be complete when we're done adding
				// all files, and it won't be used before that.
		uio_GPDir_commitSubDir(gPDir, buf, newGPDir);

		gPDir = newGPDir;
		getNextPathComponent(pathEnd, &start, &end);
	}

	uio_free(buf);
	
	file = uio_GPFile_new(gPDir->pRoot, (uio_GPFileExtra) gPFileData,
			uio_gPFileFlagsFromPRootFlags(gPDir->pRoot->flags));
	uio_GPDir_addFile(gPDir, rest, file);
	return 0;
}

static inline int
zip_foundDir(uio_GPDir *gPDir, const char *path, ziptree_GPDirData *gPDirData) {
	size_t pathLen;
	const char *pathEnd;
	const char *rest;
	const char *start, *end;
	char *buf;

	if (path[0] == '/')
		path++;
	pathLen = strlen(path);
	pathEnd = path + pathLen;

	switch (uio_walkGPPath(gPDir, path, pathLen, &gPDir, &rest)) {
		case 0:
			// The dir already exists. Only need to add gPDirData
			if (gPDir->extra != NULL) {
				fprintf(stderr, "Warning: '%s' is present more than once "
						"in the zip file. The last entry will be used.\n",
						path);
				ziptree_GPDirData_delete(gPDir->extra);
			}
			gPDir->extra = gPDirData;
			return 0;
		case ENOTDIR:
			fprintf(stderr, "Warning: A component of '%s' is not a "
					"directory - file skipped.\n", path);
			errno = ENOTDIR;
			return -1;
		case ENOENT:
			break;
	}

	buf = uio_malloc(pathLen + 1);
	getFirstPathComponent(rest, pathEnd, &start, &end);
	while (start < pathEnd) {
		uio_GPDir *newGPDir;
		
		if (end == start || (end - start == 1 && start[0] == '.') ||
				(end - start == 2 && start[0] == '.' && start[1] == '.')) {
			fprintf(stderr, "Warning: directory '%s' has an invalid path - "
					"skipped.\n", path);
			uio_free(buf);
			errno = EINVAL;
			return -1;
		}
		memcpy(buf, start, end - start);
		buf[end - start] = '\0';
		newGPDir = uio_GPDir_prepareSubDir(gPDir, buf);
		newGPDir->flags |= uio_GPDir_COMPLETE;
				// It will be complete when we're done adding
				// all files, and it won't be used before that.
		uio_GPDir_commitSubDir(gPDir, buf, newGPDir);

		gPDir = newGPDir;
		getNextPathComponent(pathEnd, &start, &end);
	}
	gPDir->extra = gPDirData;

	uio_free(buf);
	return 0;
}

static inline ziptree_GPFileData *
ziptree_GPFileData_new(void) {
	return ziptree_GPFileData_alloc();
}

static inline void
ziptree_GPFileData_delete(ziptree_GPFileData *gPFileData) {
	ziptree_GPFileData_free(gPFileData);
}

static inline ziptree_GPFileData *
ziptree_GPFileData_alloc(void) {
	ziptree_GPFileData *result = uio_malloc(sizeof (ziptree_GPFileData));
#ifdef uio_MEM_DEBUG
	uio_MemDebug_debugAlloc(ziptree_GPFileData, (void *) result);
#endif
	return result;
}

static inline void
ziptree_GPFileData_free(ziptree_GPFileData *gPFileData) {
#ifdef uio_MEM_DEBUG
	uio_MemDebug_debugFree(ziptree_GPFileData, (void *) gPFileData);
#endif
	uio_free(gPFileData);
}

static inline void
ziptree_GPDirData_delete(ziptree_GPDirData *gPDirData) {
	ziptree_GPDirData_free(gPDirData);
}

static inline void
ziptree_GPDirData_free(ziptree_GPDirData *gPDirData) {
#ifdef uio_MEM_DEBUG
	uio_MemDebug_debugFree(ziptree_GPFileData, (void *) gPDirData);
#endif
	uio_free(gPDirData);
}



