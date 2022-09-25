#!/usr/bin/usr python3
from setuptools import Extension, setup

from distutils.core import setup, Extension #这里要用到distutils库
module = Extension('opsplash', sources = ['src/pyopsplash.c'],
                     extra_compile_args=['-Wall',
                                         '-O2'],
                     include_dirs=["include"],
                     libraries=['z']  # zlib
                     )
setup (name = 'opsplash', #打包名
       version = '1.14', #版本
       description = 'opsplash Python/C lib', #说明文字
       ext_modules = [module])
