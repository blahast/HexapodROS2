from setuptools import find_packages
from setuptools import setup

setup(
    name='hexapod_custom_msgs',
    version='0.0.1',
    packages=find_packages(
        include=('hexapod_custom_msgs', 'hexapod_custom_msgs.*')),
)
