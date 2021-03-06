#! /bin/bash

if [ "$1" = --description ] ; then
  echo "create a tgz file containing a plumed release, only works from git repository"
  exit 0
fi

if [ $# != 1 ] ;
then
  echo "Syntax:"
  echo "release.sh 2.0"
  exit 1
fi

version=$1

case "$version" in
(2.?*)
  version="${version#2.}"
  ;;
(*)
  echo "ERROR"
  echo "Use a version in form 2.*"
  exit 1
  ;;
esac

ls src README 1>/dev/null 2>/dev/null ||  {
  echo "Launch from root directory"
  exit 1
}

echo "Version: 2.$version"

git archive -o plumed-2.$version.tgz --prefix plumed-2.$version/ v2.$version
