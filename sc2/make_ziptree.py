#!/usr/bin/env python
import os
from os import makedirs
from pathlib import Path
from zipfile import ZipFile, ZIP_DEFLATED, ZipInfo

out_path = Path('ziptree')

for root, dirs, files in os.walk('content'):
    assert not root.startswith('/')
    assert not root.endswith('/')
    zip_path = out_path / (root.removeprefix('content/') + '.zip')
    makedirs(zip_path.parent, exist_ok=True)

    zipfile = ZipFile(zip_path, 'w', ZIP_DEFLATED)
    for dirname in sorted(dirs):
        zif = ZipInfo(dirname + "/")
        zif.external_attr = 0x10
        zif.internal_attr = 0x10
        zif.flag_bits = 0x10
        zif.reserved = 0x10
        zipfile.writestr(zif, b'')
    for filename in sorted(files):
        zipfile.write(Path(root) / filename, arcname=filename)
    zipfile.close()

    print(f"{zip_path}: {len(dirs)} dirs {len(files)} files")
