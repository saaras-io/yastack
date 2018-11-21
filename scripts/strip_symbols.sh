#!/bin/bash

scriptdir=`dirname ${0}`
scriptdir=`(cd ${scriptdir}; pwd)`
scriptname=`basename ${0}`

set -e

function errorexit()
{
  errorcode=${1}
  shift
  echo $@
  exit ${errorcode}
}

function usage()
{
  echo "USAGE ${scriptname} <file_to_tostrip> <debug_output_dir>"
}

tostripdir=`dirname "$1"`
tostripfile_name=`basename "$1"`
tostripfile="$1"
debug_dir="$2"


if [ -z ${tostripfile} ] ; then
  usage
  errorexit 0 "<file_to_strip> must be specified"
fi

if [ -z ${debug_dir} ] ; then
  usage
  errorexit 0 "<debug_output_dir> must be specified"
fi

debugdir=${debug_dir}
debugfile="${tostripfile_name}.debug"

#if [ ! -d "${debugdir}" ] ; then
#  echo "creating dir ${tostripdir}/${debugdir}"
#  mkdir -p "${debugdir}"
#fi
echo "stripping ${tostripfile}, putting debug info into ${debugfile}"
objcopy --only-keep-debug "${tostripfile}" "${debugdir}/${debugfile}"
strip --strip-debug --strip-unneeded "${tostripfile}"
objcopy --add-gnu-debuglink="${debugdir}/${debugfile}" "${tostripfile}"
chmod -x "${debugdir}/${debugfile}"
