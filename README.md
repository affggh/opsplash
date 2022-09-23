# opsplash
A command line tool unpack/repack oppo/realme/oneplus splash image

## How to compile
### Main program
*** Need zlib-devel    
``` sh
make
```
### As python library
``` sh
python3 setup.py build
python3 setup.py install
```

## Prebuilt library
In prebuilt dir    
Have win64 python3.10 library    
Have win64 dll library

## Usage
### Unpack oppo splash image    
``` sh
./opsplash unpack -i splash.img -o pic
```
    
### Repack oppo splash image
``` sh
./opsplash repack -i splash.img -o new-splash.img
```

### Only read image info
``` sh
./opsplash readinfo -i splash.img
```