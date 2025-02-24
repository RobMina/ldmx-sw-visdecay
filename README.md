<p align="center">
    <img src="https://github.com/LDMX-Software/ldmx-software.github.io/blob/trunk/src/img/ldmx_logo_dark.png" width="500">
</p>

<p align="center">
    Simulation and reconstruction framework for the Light Dark Matter eXperiment.  
</p>

<p align="center">
    <a href="http://perso.crans.org/besson/LICENSE.html" alt="GPLv3 license">
        <img src="https://img.shields.io/badge/License-GPLv3-blue.svg" />
    </a>
    <a href="https://github.com/LDMX-Software/ldmx-sw/actions/workflows/build_production_image.yml" alt="Build Production Image">
        <img src="https://github.com/LDMX-Software/ldmx-sw/actions/workflows/build_production_image.yml/badge.svg"/>
    </a>
    <img src="https://github.com/LDMX-Software/ldmx-sw/actions/workflows/basic_test.yml/badge.svg" />
</p>

## Quick Start

- [Install the docker engine](https://docs.docker.com/engine/install/)
- (on Linux systems) [Manage docker as non-root user](https://docs.docker.com/engine/install/linux-postinstall/#manage-docker-as-a-non-root-user)
- (on MacOS systems) Make sure `git lfs` is installed. (Test: `git lfs` prints out a help message instead of an error about `lfs` not being found.)
  - The default installation of `git` that is included with Apple's developer tools does not include `git lfs` which is required by acts to download and unpack one of its own submodules. [GitHub has a nice tutorial](https://docs.github.com/en/repositories/working-with-files/managing-large-files/installing-git-large-file-storage?platform=mac) on how to install `git lfs` on MacOS.
  - This is only an issue observed on MacOS systems. Linux repositories (used in WSL and Linux systems) include `git lfs` within their installed versions of `git`.
- Clone the repo: `git clone --recursive git@github.com:LDMX-Software/ldmx-sw.git`
  - **Note**: You need to [setup an SSH-key with your GitHub account](https://docs.github.com/en/authentication/connecting-to-github-with-ssh) on the computer you are using.
- Setup the environment (in bash): `source ldmx-sw/scripts/ldmx-env.sh`
  - **Note**: If you are working with ldmx-sw at SLAC's SDF, you will need to set the `TMPDIR` environment variable so that program running the container has more than ~5GB of space to write intermediate files. The default temporary space (`/tmp`) is often full of other files already. A decent replacement is `TMPDIR=${SCRATCH}` which gives the program plenty of room for the files it needs to manipulate.
- Configure and compile: `ldmx compile`
- Now you can run any processor in _ldmx-sw_ through `ldmx fire myconfig.py`
- If you are developing and need to recompile and run `ldmx fire`, you can use `ldmx recompFire myconfig.py`

## Documentation 
The full documentation for **ldmx-sw** is available on [github pages](https://ldmx-software.github.io/).
A brief description of common commands is given below.

### Common Commands inside Container

Command | Purpose
---|---
`ldmx cmake ..` | Configure the ldmx-sw build
`ldmx make` | Compile/build ldmx-sw
`ldmx make install` | Install ldmx-sw
`ldmx compile` | Configure and compile ldmx-sw
`ldmx fire config.py` | Use ldmx-sw application and processors with input python configuration
`ldmx recompFire config.py` | Recompile and run fire on a config file
`ldmx python3 analysis.py` | Run python-based analysis
`ldmx ./bin/mg5_aMC` | Run MadGraph5 inside (ubuntu-based) container

### Other Container Configuration Commands

The environment script defines several other shell commands to help configure and debug the container environment.

- `ldmx list repo` : List the container tags that you could use with the input repository: `dev`, `pro`, or `local`
- `ldmx use repo tag` : Setup the environment for the container 'ldmx/repo:tag' and pull down the newest version if the repo is remote
- `ldmx config` : Print out how the container environment is currently configured
- `ldmx clean all` : Reset environment to a blank state

Use `ldmx help` for a full listing of these commands.
If we don't define a command outside of the container,
then the command is given to the container to run inside the current directory.

## Maintainer 

## Contributors

<a href="https://github.com/LDMX-Software/ldmx-sw/graphs/contributors">
  <img src="https://contributors-img.web.app/image?repo=LDMX-Software/ldmx-sw" />
</a>

