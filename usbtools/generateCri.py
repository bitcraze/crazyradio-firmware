#!/usr/bin/env python2

import sys
import hashlib
import zipfile
import json
import os

HASH_ALGO = "sha256"

usage = """Bitcraze Crazyradio Image generator
   {} <binary_image> <output.cri> [verified(0|1)]\n""".format(sys.argv[0])

def generate_manifest(verified):
    man = {}
    
    with os.popen("hg id -ibt") as fd:
        identity = fd.read().split(" ")
    man['version'] = identity[0].strip()
    man['branch'] = identity[1].strip()
    if len(identity)>2:
        man['tag'] = identity[2].strip()
    else:
        man['tag'] = ''
    man['verified'] = verified
    
    return man

def main():
    if len(sys.argv)<3 or len(sys.argv)>4:
        sys.stderr.write(usage)
        return 1
    
    verified = False
    if len(sys.argv)==4 and sys.argv[3]=='1':
        print("Verified image!")
        verified=True
    else:
        print("Development image")
    
    try:
        with open(sys.argv[1]) as imagefile:
            image = imagefile.read()
    except:
        sys.stderr.write("Cannot open {}.\n".format(sys.argv[1]))
        return 2
    
    try:
        cri = zipfile.ZipFile(sys.argv[2], "w")
    except:
        sys.stderr.write("Cannot create {}.\n".format(sys.argv[2]))
        return 3
    
    manifest = json.dumps(generate_manifest(verified))
    
    cri.writestr("image.bin", image)
    cri.writestr("manifest.json", manifest)
    
    himage = hashlib.new(HASH_ALGO)
    himage.update(image)
    himage.update(manifest)
    cri.writestr("hash", "{}:{}".format(himage.name, himage.hexdigest()))
    
    print("Created cri with hash {}:{}...".format(himage.name,
            himage.hexdigest()[0:8]))
    
    cri.close()
    return 0

if __name__=="__main__":
    sys.exit(main())
