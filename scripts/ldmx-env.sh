#!/bin/bash

###############################################################################
# ldmx-env.sh
#   This script is intended to define all the container aliases required
#   to develop for ldmx-sw. These commands assume that the user
#     1. Has docker engine installed OR has singularity installed
#     2. Can run docker as a non-root user OR can run singularity build/run
#
#   SUGGESTION: Put something similar to the following in your '.bashrc',
#     '~/.bash_aliases', or '~/.bash_profile' so that you just have to 
#     run 'ldmx-env' to set-up this environment.
#
#   alias ldmx-env='source <full-path>/ldmx-env.sh; unalias ldmx-env'
#
#   The file 'ldmx-sw/.ldmxrc' handles the default environment setup for the
#   container. Look there for persisting your custom settings.
###############################################################################

###############################################################################
# All of this setup requires us to be in a bash shell.
#   We add this check to make sure the user is in a bash shell.
###############################################################################
if [[ -z ${BASH} ]]; then
  echo "[ldmx-env.sh] [ERROR] You aren't in a bash shell. You are in '$0'."
  [[ "$SHELL" = *"bash"* ]] || echo "  You're default shell '$SHELL' isn't bash."
  return 1
fi

###############################################################################
# __ldmx_has_required_engine
#   Checks if user has any of the supported engines for running containers
###############################################################################
__ldmx_has_required_engine() {
  if hash docker &> /dev/null; then
    return 0
  elif hash singularity &> /dev/null; then
    return 0
  else
    return 1
  fi
}

# check if user has a required engine
if ! __ldmx_has_required_engine; then
  echo "[ldmx-env.sh] [ERROR] You do not have docker or singularity installed!"
  return 1
fi

###############################################################################
# __ldmx_which_os
#   Check what OS we are hosting the container on.
#   Taken from https://stackoverflow.com/a/8597411
#   and to integrate Windoze Subsystem for Linux: 
#     https://wiki.ubuntu.com/WSL#Running_Graphical_Applications
###############################################################################
export LDMX_CONTAINER_DISPLAY=""
__ldmx_which_os() {
  if uname -a | grep -q microsoft; then
    # Windoze Subsystem for Linux
    export LDMX_CONTAINER_DISPLAY=$(awk '/nameserver / {print $2; exit}' /etc/resolv.conf 2>/dev/null)    
    return 0
  elif [[ "$OSTYPE" == "darwin"* ]]; then
    # Mac OSX
    export LDMX_CONTAINER_DISPLAY="docker.for.mac.host.internal"
    return 0
  elif [[ "$OSTYPE" == "linux"* || "$OSTYPE" == "freebsd"* ]]; then
    # Linux/BSD distribution
    export LDMX_CONTAINER_DISPLAY=""
    return 0
  fi

  return 1
}

if ! __ldmx_which_os; then
  echo "[ldmx-env.sh] [WARN] Unable to detect OS Type from '${OSTYPE}' or '$(uname -a)'"
  echo "    You will *not* be able to run display-connected programs."
fi

###############################################################################
# We have gotten here after determining that we definitely have a container
# runner (either docker or singularity) and we have determined how to connect
# the display (or warn the user that we can't) via the LDMX_CONTAINER_DISPLAY
# variable.
#
#   All container-runners need to implement the following commands
#     - __ldmx_list_local : list images available locally
#     - __ldmx_use : setup the environment to use the specified container
#         - Three arguments: <repo> <tag> <pull_no_matter_what>
#     - __ldmx_run : give all arguments to container's entrypoint script
#         - mounts all directories in bash array LDMX_CONTAINER_MOUNTS
#         - sets all environment variables in bash array LDMX_CONTAINER_ENVS
#     - __ldmx_container_clean : remove all containers and images on this machine
#     - __ldmx_container_config : print configuration of container
###############################################################################

# prefer docker, so we do that first
if hash docker &> /dev/null; then
  # List containers on our machine matching the sub-string 'ldmx/local'
  __ldmx_list_local() {
    docker images "ldmx/local"
  }

  # Use the input container and error out if not available
  __ldmx_use() {
    local _repo_name="$1"
    local _image_tag="$2"
    local _pull_down="$3"
    export LDMX_DOCKER_TAG="ldmx/${_repo_name}:${_image_tag}"
    if [ -z "$(docker images -q ${LDMX_DOCKER_TAG} 2> /dev/null)" ]; then
      echo "No local docker image matching the name '${LDMX_DOCKER_TAG}'."
      if [ "${_repo_name}" == "local" ]; then
        echo "  You can add another tag to your local image and match our required format:"
        echo "      docker tag my-image-tag ldmx/local:my-image-tag"
        echo "  Then you can use"
        echo "      ldmx use local my-image-tag"
      else
        echo "Downloading..."
        docker pull ${LDMX_DOCKER_TAG}
        return $?
      fi
      return 1
    elif [ ! -z ${_pull_down} ]; then
      echo "Downloading..."
      docker pull ${LDMX_DOCKER_TAG}
      return $?
    fi

    return 0
  }

  # Print container configuration
  #   SHA retrieval taken from https://stackoverflow.com/a/33511811
  __ldmx_container_config() {
    echo "Docker Version: $(docker --version)"
    echo "Docker Tag: ${LDMX_DOCKER_TAG}"
    echo "    Digest: $(docker inspect --format='{{index .RepoDigests 0}}' ${LDMX_DOCKER_TAG})"
    return 0
  }

  # Clean up local machine
  __ldmx_container_clean() {
    docker container prune -f || return $?
    docker image prune -a -f  || return $?
  }

  # Run the container
  __ldmx_run() {
    local _mounts=""
    for dir_to_mount in "${LDMX_CONTAINER_MOUNTS[@]}"; do
      _mounts="$_mounts -v $dir_to_mount:$dir_to_mount"
    done
    local _envs=""
    for env_to_set in ${LDMX_CONTAINER_ENVS[@]}; do
      _envs="$_envs -e ${env_to_set}"
    done
	local interactive=""
    tty -s && interactive="-it"
    docker run --rm ${interactive} \
      -e LDMX_BASE \
      -e DISPLAY=${LDMX_CONTAINER_DISPLAY}:0 \
      $_envs \
      -v /tmp/.X11-unix:/tmp/.X11-unix \
      $_mounts \
      -u $(id -u ${USER}):$(id -g ${USER}) \
      $LDMX_DOCKER_TAG "$@"
    return $?
  }
elif hash singularity &> /dev/null; then
  # List all '.sif' files in LDMX_BASE directory
  __ldmx_list_local() {
    ls -Alh ${LDMX_BASE} | grep ".*local.*sif"
    return 0
  }

  # Use the input container in your workflow
  __ldmx_use() {
    local _repo_name="$1"
    local _image_tag="$2"
    local _pull_down="$3"

    # change cache directory to be inside ldmx base directory
    export SINGULARITY_CACHEDIR=${LDMX_BASE}/.singularity
    mkdir -p ${SINGULARITY_CACHEDIR} #make sure cache directory exists
  
    # name the singularity image after the tag the user asked for
    export LDMX_SINGULARITY_IMG=ldmx_${_repo_name}_${_image_tag}.sif
  
    if [ ! -f "${LDMX_BASE}/${LDMX_SINGULARITY_IMG}" ]; then
      echo "No local singularity image at '${LDMX_BASE}/${LDMX_SINGULARITY_IMG}'."
      if [ "${_repo_name}" == "local" ]; then
        echo "  You can point ldmx to your singularity image in the correct format using symlinks."
        echo "      cd $LDMX_BASE"
        echo "      ln -s <path-to-local-singularity-image>.sif ldmx_local_my-image-tag.sif"
        echo "  Then you can use"
        echo "      ldmx-container-pull local my-image-tag"
      else
        echo "Downloading..."
        singularity build \
          --force \
          ${LDMX_BASE}/${LDMX_SINGULARITY_IMG} \
          docker://ldmx/${_repo_name}:${_image_tag}
        return $?
      fi
      return 1
    elif [ ! -z $_pull_down ]; then
      singularity build \
        --force \
        ${LDMX_BASE}/${LDMX_SINGULARITY_IMG} \
        docker://ldmx/${_repo_name}:${_image_tag}
      return $?
    fi
    return 0
  }

  # Print container configuration
  __ldmx_container_config() {
    echo "Singularity Version: $(singularity version)"
    echo "Singularity File: ${LDMX_BASE}/${LDMX_SINGULARITY_IMG}"
    return 0
  }

  # Clean up local machine
  __ldmx_container_clean() {
    rm $LDMX_BASE/*.sif || return $?
    rm -r $SINGULARITY_CACHEDIR || return $?
  }

  # Run the container
  __ldmx_run() {
    local csv_list="/tmp/.X11-unix"
    for dir_to_mount in "${LDMX_CONTAINER_MOUNTS[@]}"; do
      csv_list="$dir_to_mount,$csv_list"
    done
    local env_list
    for env_to_set in ${LDMX_CONTAINER_ENVS[@]}; do
      env_list="${env_list},${env_to_set}" 
    done
    singularity run --no-home --cleanenv \
      --env LDMX_BASE=${LDMX_BASE},DISPLAY=${LDMX_CONTAINER_DISPLAY}:0${env_list} \
      --bind ${csv_list} ${LDMX_SINGULARITY_IMG} "$@"
    return $?
  }
fi

###############################################################################
# __ldmx_list
#   Get the docker tags for the repository
#   Taken from https://stackoverflow.com/a/39454426
# If passed repo-name is 'local',
#   the list of container options is runner-dependent
###############################################################################
__ldmx_list() {
  _repo_name="$1"
  if [ "${_repo_name}" == "local" ]; then
    __ldmx_list_local
    return $?
  else
    #line-by-line description
    # download tag json
    # strip unnecessary information
    # break tags into their own lines
    # pick out tags using : as separator
    # put tags back onto same line
    wget -q https://registry.hub.docker.com/v1/repositories/ldmx/${_repo_name}/tags -O -  |\
        sed -e 's/[][]//g' -e 's/"//g' -e 's/ //g' |\
        tr '}' '\n'  |\
        awk -F: '{print $3}' |\
        tr '\n' ' '
    local rc=${PIPESTATUS[0]}
    echo "" #new line
    return ${rc}
  fi
}

###############################################################################
# __ldmx_config
#   Print the configuration of the current setup
###############################################################################
__ldmx_config() {
  echo "LDMX base directory: ${LDMX_BASE}"
  echo "uname: $(uname -a)"
  echo "OSTYPE: ${OSTYPE}"
  echo "Bash version: ${BASH_VERSION}"
  echo "Display Port: ${LDMX_CONTAINER_DISPLAY}"
  echo "Container Mounts: ${LDMX_CONTAINER_MOUNTS[@]}"
  echo "Container Environments: ${LDMX_CONTAINER_ENVS[@]}"
  __ldmx_container_config
  return $?
}

###############################################################################
# __ldmx_is_mounted
#   Check if the input directory will be accessible by the container
###############################################################################
__ldmx_is_mounted() {
  local full=$(cd "$1" && pwd -P)
  for _already_mounted in ${LDMX_CONTAINER_MOUNTS[@]}; do
    if [[ $full/ = $_already_mounted/* ]]; then
      return 0
    fi
  done
  return 1
}

###############################################################################
# __ldmx_run_here
#   Call the run method with some fancy directory movement around it
###############################################################################
__ldmx_run_here() {
  #store last directory to resume history later
  local _old_pwd=$OLDPWD
  #store current working directory relative to ldmx base
  local _pwd=$(pwd -P)/.

  # check if container will be able to see where we are
  if ! __ldmx_is_mounted $_pwd; then
    echo "You aren't in a directory mounted to the container!"
    return 1
  fi

  cd ${LDMX_BASE} # go to ldmx base directory outside container
  # go to working directory inside container
  __ldmx_run $_pwd "$@"
  local rc=$?
  cd - &> /dev/null
  export OLDPWD=$_old_pwd
  return ${rc}
}

###############################################################################
# __ldmx_mount
#   Tell us to mount the passed directory to the container when we run
#   By default, we already mount the LDMX_BASE directory, so none of
#   its subdirectories need to (or should be) specified.
###############################################################################
export LDMX_CONTAINER_MOUNTS=()
__ldmx_mount() {
  local _dir_to_mount="$1"
  
  if [[ ! -d $_dir_to_mount ]]; then
    echo "$_dir_to_mount is not a directory!"
    return 1
  fi

  if __ldmx_is_mounted $_dir_to_mount; then
    echo "$_dir_to_mount is already mounted"
    return 0
  fi

  LDMX_CONTAINER_MOUNTS+=($(cd "$_dir_to_mount" && pwd -P))
  export LDMX_CONTAINER_MOUNTS
  return 0
}


###############################################################################
# __ldmx_base
#   Define the base directory of ldmx software
###############################################################################
export LDMX_BASE=""
__ldmx_base() {
  local _new_base="$1"
  if [[ ! -d $_new_base ]]; then
    echo "'$_new_base' is not a directory!"
    return 1
  fi

  export LDMX_BASE=$(cd $_new_base; pwd -P)
  __ldmx_mount $LDMX_BASE
  return $?
}

###############################################################################                                                                    
# __ldmx_setenv
#   Tell us to pass an environment variable to the container when we run
#   By default, we pass the LDMX_BASE and DISPLAY variables explicitly, 
#   because their syntax is too different between docker and singularity,
#   so none of these need to (or should be) specified.
###############################################################################

export LDMX_CONTAINER_ENVS=()
__ldmx_setenv() {
  local _env_to_set="$1"

  if [[ $_env_to_set != *"="* ]]; then
    echo "$_env_to_set doesn't follow the required syntax myEnv=someValue!"
	  echo "Not setting container environment variable $_env_to_set."
    return 1
  fi

  local envName=$(echo $_env_to_set | cut -d= -f1)
  local KEY=$(echo $_env_to_set | cut -d= -f1)
  local VALUE=$(echo $_env_to_set | cut -d= -f2-) # Field 2 and any further fields
  local key_exists=false

  for entry in "${!LDMX_CONTAINER_ENVS[@]}"; do
    entry_key=$(echo "${LDMX_CONTAINER_ENVS[$entry]}" | cut -d= -f1)
    if [ -z "${entry_key}" ]; then
      continue
    fi
    if [ "${KEY}" == "${entry_key}" ]; then
      echo "Updating environment variable ${KEY}: ${LDMX_CONTAINER_ENVS[$entry]} -> ${KEY}=${VALUE}"
      LDMX_CONTAINER_ENVS[$entry]="${KEY}=${VALUE}"
      key_exists=true
      break
    fi
  done
  if [ "${key_exists}" == "false" ]; then
    LDMX_CONTAINER_ENVS+=("${KEY}=${VALUE}")
  fi


  # Loop over the indices of the array
  export LDMX_CONTAINER_ENVS
  echo "Added container environment variable $_env_to_set"
  return 0
}


###############################################################################
# __ldmx_clean
#   Clean up the computing environment for ldmx
#   The input argument defines what should be cleaned
###############################################################################
__ldmx_clean() {
  local _what="$1"
  local cleaned_something=false
  local rc=0
  if [[ "$_what" = "container" ]] || [[ "$_what" = "all" ]]; then
    __ldmx_container_clean
    cleaned_something=true
    rc=$?
  fi

  if [[ "$_what" = "src" ]] || [[ "$_what" = "all" ]]; then
    local _old_pwd=$OLDPWD
    cd ${LDMX_BASE}/ldmx-sw
    git clean -xf
    rc=$?
    git submodule foreach git clean -xf
    rc=$?
    [[ -d build ]] && rm -r build
    [[ -d install ]] && rm -r install
    cleaned_something=true
    cd - &>/dev/null
    export OLDPWD=$_old_pwd
  fi

  # must be last so cleaning of source can look in ldmx base
  if [[ "$_what" = "env" ]] || [[ "$_what" = "all" ]]; then
    unset LDMX_BASE
    unset LDMX_CONTAINER_MOUNTS
    unset LDMX_CONTAINER_DISPLAY
    unset LDMX_CONTAINER_ENVS
    cleaned_something=true
  fi

  if ! $cleaned_something; then
    echo "ERROR: $_what is not one of the clean options."
    rc=1
  fi

  return ${rc}
}



###############################################################################
# __ldmx_source
#   Run all the sub-commands in the provided file from the directory it is in.
#   Ignore empty lines or lines starting with '#'
###############################################################################
__ldmx_source() {
  local _file_listing_commands="$1"
  local _old_pwd=$OLDPWD
  cd $(dirname $_file_listing_commands)
  while read _subcmd; do
    if [[ -z "$_subcmd" ]] || [[ "$_subcmd" = \#* ]]; then
      continue
    fi
    ldmx $_subcmd
  done < $(basename $_file_listing_commands)
  cd - &> /dev/null
  export OLDPWD=$_old_pwd
}

###############################################################################
# __ldmx_checkout
#   Wrapper around git to help with common workflow
#   We assume we are running this from the source code directory
#   User provides branch configuration they want the source code to reflect
###############################################################################
__ldmx_checkout() {
  local __ldmxsw=""
  local _submodules=()
  for __ldmx_br in $@; do
    if [[ "$__ldmx_br" == *":"* ]]; then
      # submodule specified
      _submodules+=("$__ldmx_br")
    elif [[ ! -z "$__ldmxsw" ]]; then
      echo "Can't provide more than one branch for ldmx-sw."
      return 1
    else
      __ldmxsw="$__ldmx_br"
    fi
  done

  # do ldmx-sw first so we can submodule update
  # and then change submodule branches later if necessary
  if [[ ! -z $__ldmxsw ]]; then
    git checkout $__ldmxsw || return $?
    git submodule update || return $?
  fi

  # now do submodules
  for _sub_br in ${_submodules[@]}; do
    IFS=: read -r _submodule _branch <<< "$_sub_br"
    if [[ -z $_submodule ]] || [[ -z $_branch ]]; then
      echo "Unrecognized ldmx-sw branch format: '$_sub_br'."
      echo "  Should be of the form <submodule>:<branch>."
      return 1
    fi
    echo "Checking out $_branch in $_submodule..."
    local _old_pwd=$OLDPWD
    if ! cd $_submodule; then
      echo "'$_submodule' does not exist."
      return 1
    fi
    local rc=0
    git checkout $_branch
    rc=$?
    cd - &> /dev/null
    export OLDPWD=$_old_pwd
    [[ "$rc" != "0" ]] && return ${rc}
  done

  return 0
}

###############################################################################
# __ldmx_compile
#   Compile ldmx-sw
###############################################################################

__ldmx_compile() {
  ldmx . ${LDMX_BASE}/ldmx-sw/scripts/ldmx-compile.sh
}

###############################################################################
# __ldmx_recompFire
#   Recompile ldmx-sw and run fire on the first argument
###############################################################################

__ldmx_recompFire() {
  ldmx . ${LDMX_BASE}/ldmx-sw/scripts/ldmx-recompileAndFire.sh $@
}

###############################################################################
# __ldmx_help
#   Print some helpful message to the terminal
###############################################################################
__ldmx_help() {
  cat <<\HELP

  USAGE: 
    ldmx <command> [<argument> ...]

  COMMANDS:
    help    : print this help message and exit
      ldmx help
    compile : Compile ldmx-sw
      ldmx compile
    list    : List the tag options for the input container repository
      ldmx list (dev | pro | local)
    clean   : Reset ldmx computing environment
              container - remove containers and images on computer
              env - reset environment variables
              src - remove build/install directory and auto-generated files
      ldmx clean (all | container | env | src)
    config  : Print the current configuration of the container
      ldmx config
    checkout: Checkout a specific source-code configuration for ldmx-sw
      ldmx checkout <ldmx-sw-branch> [<submodule>:<branch> ...]
      ldmx checkout <submodule>:<branch> [<submodule2>:<branch2> ...]
    use     : Use the input repo and tag of the container for running
      ldmx use (dev | pro | local) <tag>
    pull    : Pull down the input repo and tag of the container
      ldmx pull (dev | pro | local) <tag>
    mount   : Attach the input directory to the container when running
      ldmx mount <dir>
    setenv   : Set an environment variable in the container when running
      ldmx setenv <environmentVariableName=value>
    run     : Run a command at an input location in the container
      ldmx run <directory> <sub-command> [<argument> ...]
    source  : Run the commands in the provided file through ldmx
      ldmx source .ldmxrc
    <other> : Run the input command in your current directory in the container
      ldmx <other> [<argument> ...]
      ldmx cmake ..
      ldmx make install
      ldmx fire config.py
      ldmx recompFire config.py
      ldmx python3 ana.py

HELP
  return 0
}

###############################################################################
# ldmx
#   The root command for users interacting with the ldmx container environment.
#   This function is really just focused on parsing CLI and going to the
#   corresponding subcommand.
#
#   There are lots of subcommands, go to those functions to learn the detail
#   about them.
###############################################################################
ldmx() {
  # if there are no arguments, print the help
  [[ "$#" == "0" ]] && { __ldmx_help; return $?; }
  # divide commands by number of arguments
  case $1 in
    help|config|compile)
      if [[ "$#" != "1" ]]; then
        __ldmx_${1}
        echo "ERROR: 'ldmx ${1}' takes no arguments."
        return 1
      fi
      __ldmx_${1}
      return $?
      ;;
    list|base|clean|mount|setenv|source)
      if [[ "$#" != "2" ]]; then
        __ldmx_help
        echo "ERROR: ldmx ${1} takes one argument."
        return 1
      fi
      __ldmx_${1} "$2"
      return $?
      ;;
    pull)
      if [[ "$#" != "3" ]]; then
        __ldmx_help
        echo "ERROR: 'ldmx pull' takes two arguments: <repo> <tag>."
        return 1
      fi
      __ldmx_use "$2" "$3" "PULL_NO_MATTER_WHAT"
      return $?
      ;;
    use)
      if [[ "$#" != "3" ]]; then
        __ldmx_help
        echo "ERROR: 'ldmx use' takes two arguments: <repo> <tag>."
        return 1
      fi
      __ldmx_use "$2" "$3"
      return $?
      ;;
    run|checkout|recompFire)
      __ldmx_${1} ${@:2}
      return $?
      ;;
    *)
      __ldmx_run_here $@
      return $?
      ;;
  esac
}

###############################################################################
# DONE WITH NECESSARY PARTS
#   Everything below here is icing on the usability cake.
###############################################################################

###############################################################################
# Bash Tab Completion
#   This next section is focused on setting up the infrastucture for smart
#   tab completion with the ldmx command and its sub-commands.
###############################################################################

###############################################################################
# __ldmx_complete_directory
#   Some of our sub-commands take a directory as input.
#   In these cases, we can pretend to cd and use bash's internal
#   tab-complete functions.
#   
#   All this requires is for us to shift the COMP_WORDS array one to
#   the left so that the bash internal tab-complete functions don't
#   get distracted by our base command 'ldmx' at the front.
#
#   We could allow for the shift to be more than one if there is a deeper
#   tree of commands that need to be allowed in the futre.
###############################################################################
__ldmx_complete_directory() {
  local _num_words="1"
  COMP_WORDS=(${COMP_WORDS[@]:_num_words})
  COMP_CWORD=$((COMP_CWORD - _num_words))
  _cd
}

###############################################################################
# __ldmx_complete_command
#   Tab-complete with a command used commonly inside the container
#
#   Search the install location of ldmx-sw for ldmx-sw executables
#   and include hard-coded common commands. Any strings passed are
#   also included.
#
#   Assumes current argument being tab completed is stored in
#   bash variable 'curr_word'.
###############################################################################
__ldmx_complete_command() {
  # generate up-to-date list of options
  local _options="$@ cmake make python3 root rootbrowse"
  for ldmx_executable in ${LDMX_BASE}/ldmx-sw/install/bin/*; do
    _options="$_options $(basename $ldmx_executable)"
  done

  # match current word (perhaps empty) to the list of options
  COMPREPLY=($(compgen -W "$_options" "$curr_word"))
}

###############################################################################
# __ldmx_complete_bash_default
#   Restore the default tab-completion in bash that uses the readline function
#   Bash default tab completion just looks for filenames
###############################################################################
__ldmx_complete_bash_default() {
  compopt -o default
  COMPREPLY=()
}

###############################################################################
# __ldmx_dont_complete
#   Don't tab complete or suggest anything if user <tab>s
###############################################################################
__ldmx_dont_complete() {
  COMPREPLY=()
}

###############################################################################
# __ldmx_complete_branch
#   Tab complete the __ldmx_checkout command.
#   Tries to make current word match an ldmx-sw branch name or the
#   <submodule>:<branch> format
###############################################################################
__ldmx_complete_branch() {
  local _curr_word="$1"

  alias _get_git_branch_list='git branch | sed "s/\*//"'

  local _submodules=($(git config --file .gitmodules --get-regexp path | awk '{print $2}'))
  for _submod in ${_submodules[@]}; do
    if [[ "$_curr_word" == "$_submod:"* ]]; then
      _sub_mod_branches=($(cd $_submod && compgen -W "$(_get_git_branch_list)" "${_curr_word#*:}"))
      if (( ${#_sub_mod_branches[@]} == 1 )); then
        COMPREPLY=("${_submod}:${_sub_mod_branches[0]}")
      else
        COMPREPLY=(${_sub_mod_branches[@]})
      fi
      break
    fi
  done

  _options="$(_get_git_branch_list) ${_submodules[@]}"
  COMPREPLY=($(compgen -W "${_options}" "$_curr_word"))

  if (( ${#COMPREPLY[@]} == 1 )); then
    local _match=${COMPREPLY[0]}  
    for _submod in ${_submodules[@]}; do
      if [[ $_match == $_submod ]]; then
        COMPREPLY=("${_match}:")
        break
      fi
    done
  fi
}

###############################################################################
# Modify the list of completion options on the command line
#   Helpful discussion of this procedure from a blog post
#   https://iridakos.com/programming/2018/03/01/bash-programmable-completion-tutorial
#
#   Helpful Stackoverflow answer
#   https://stackoverflow.com/a/19062943
#
#   COMP_WORDS - bash array of space-separated command line inputs including base command
#   COMP_CWORD - index of current word in argument list
#   COMPREPLY  - options available to user, if only one, auto completed
###############################################################################
__ldmx_complete() {
  # disable readline filename completion
  compopt +o default

  local curr_word="${COMP_WORDS[$COMP_CWORD]}"

  if [[ "$COMP_CWORD" = "1" ]]; then
    # tab completing a main argument
    __ldmx_complete_command "list clean config checkout pull use run mount setenv base source compile recompFire"
  elif [[ "$COMP_CWORD" = "2" ]]; then
    # tab complete a sub-argument,
    #   depends on the main argument
    case "${COMP_WORDS[1]}" in
      config|setenv|compile)
        # no more arguments
        __ldmx_dont_complete
        ;;
      checkout)
        __ldmx_complete_branch "$curr_word"
        ;;
      clean)
        # arguments from special set
        COMPREPLY=($(compgen -W "all container env src" "$curr_word"))
        ;;
      list|pull|use)
        # container repositories after these commands
        COMPREPLY=($(compgen -W "dev pro local" "$_curr_word"))
        ;;
      run|mount|base)
        #directories only after these commands
        __ldmx_complete_directory
        ;;
      *)
        # files like normal tab complete after everything else
        __ldmx_complete_bash_default
        ;;
    esac
  else
    # three or more arguments
    #   check base argument to see if we should continue
    case "${COMP_WORDS[1]}" in
      list|base|clean|config|pull|use|mount|setenv|source)
        # these commands shouldn't have tab complete for the third argument
        #   (or shouldn't have the third argument at all)
        __ldmx_dont_complete
        ;;
      checkout)
        __ldmx_complete_branch "$curr_word"
        ;;
      run)
        if [[ "$COMP_CWORD" = "3" ]]; then
          # third argument to run should be an inside-container command
          __ldmx_complete_command
        else
          # later arguments to run should be bash default
          __ldmx_complete_bash_default
        fi
        ;;
      *)
        # everything else has bash default (filenames)
        __ldmx_complete_bash_default
        ;;
    esac
  fi
}

# Tell bash the tab-complete options for our main function ldmx
complete -F __ldmx_complete ldmx

###############################################################################
# If the default environment file exists, source it.
# Otherwise, trust that the user knows what they are doing.
###############################################################################

#default backs out of scripts/
_default_location_ldmxrc="$( dirname ${BASH_SOURCE[0]} )/../.ldmxrc"

if [[ -f $_default_location_ldmxrc ]]; then
  ldmx source $_default_location_ldmxrc
fi
