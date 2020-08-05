import os
import shutil
from distutils.dir_util import copy_tree

from setuptools import find_packages, setup

# global variables
board = os.environ['BOARD']
repo_board_folder = f'Boards/PYNQ-Z2'
board_notebooks_dir = os.environ['PYNQ_JUPYTER_NOTEBOOKS']
board_project_dir = os.path.join(board_notebooks_dir, 'Handwritten_Mathematical_Calculator_on_FPGA')

# check whether board is supported
def check_env():
    if not board == 'Pynq-Z2':
        raise ValueError("Board {} is not supported.".format(board))
        
# check if the path already exists, delete if so
def check_path():
    if os.path.exists(board_project_dir):
        shutil.rmtree(board_project_dir)

# copy overlays to python package
def copy_overlays():
    src_ol_dir = os.path.join(repo_board_folder, 'hw_bd')
    dst_ol_dir = os.path.join(board_project_dir, 'hw_bd')
    copy_tree(src_ol_dir, dst_ol_dir)

# copy notebooks to jupyter home
def copy_notebooks():
    src_nb_dir = os.path.join(repo_board_folder, 'jupyter_notebooks')
    dst_nb_dir = os.path.join(board_project_dir)
    copy_tree(src_nb_dir, dst_nb_dir)
    
check_env()
check_path()
copy_overlays()
copy_notebooks()

setup(
    name="Handwritten_Mathematical_Calculator_on_FPGA",
    version='1.0',
    install_requires=[
        'pynq=2.5',
    ],
    url='https://github.com/Microdent/Handwritten_Mathematical_Calculator_on_FPGA',
    license='BSD 3-Clause License',
    author="Quan Deng, Yun Wang, Zhiteng Chao",
    author_email="dengquan1998@tju.edu.cn",
    packages=find_packages(),
    description="A simple demo to implement the Handwritten Mathematical Calculator on PYNQ-Z2 FPGA platform by using HLS.")