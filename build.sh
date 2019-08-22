#!/usr/bin/env bash
set -e
set -o pipefail

root=$(dirname $(readlink -f $0))
processor=$root/asciidoctor_wrapper.rb
stylesheet=$root/lepota/mastercopy/lepota.min.css

modules="
  $root/lepota
  $root/ph/ep1-join
  $root/ph/ep2-select
  $root/ph/ep3-stability
  $root/ph/ep4-faq
  $root/scrumdysf
  $root/smine
  $root/srvsup
  $root/sspar
  $root/bm
  $root/inline
  $root/tcp
  $root/thumbnail
  $root/codesearch
  $root/sim/minhash1
  $root/lxcdeb
"
if [ -L $0 ]; then
  modules=$(readlink -f $PWD)
fi

cmd=build
if [ "$1" == 'build' ] || [ "$1" == '' ]; then
  cmd='build'
elif [ "$1" == 'clean' ]; then
  cmd='clean'
else
  echo "ERROR: unknown command '$1'"
  exit 1
fi


#build <target> <command> <dependency>...
function build {
  local target="$1"; shift
  local cmd="$1"; shift
  local deps="$*"

  local rebuild=0
  if ! [ -e "$target" ]; then
    rebuild=1
  else
    for dep in $deps; do
      if [ "$target" -ot "$dep" ]; then
        rebuild=1
        break
      fi
    done
  fi

  if [ $rebuild -eq 1 ]; then
    echo $cmd
    bash -c "$cmd"
  fi
}

function build_module {
  local module=$(readlink -f "$1")
  echo "Building $module"

  if [ "$module" == "$root/lepota" ]; then
    local scss=$module/src/lepota.scss
    local css=$module/mastercopy/lepota.css
    local mincss=$module/mastercopy/lepota.min.css

    build $css "sassc -I $module/src $scss $css" $scss
    build $mincss "yui-compressor -o $mincss $css" $css
  fi

  local txt="${module}/src/index.txt"
  local html="${module}/mastercopy/index.html"
  local images=$(sed -rn '/^image::(.*(\.svg|\.png|\.jpg|\.gif)).*$$/s//\1/p' ${txt})

  build $html \
    "TZ=UTC MATH_OUTPUT=${module}/mastercopy ${processor} -a stylesheet=${stylesheet} -o $html $txt" \
    $txt $processor $stylesheet

  for img in $images; do
    dia=${img//.[a-z][a-z][a-z]/.dia}
    if [ -e $module/src/$img ]; then
      build $module/mastercopy/$img "cp $module/src/$img $module/mastercopy/$img" $module/src/$img
    elif [ -e $module/src/$dia ]; then
      build $module/mastercopy/$img "dia -s 1600x -e $module/mastercopy/$img $module/src/$dia" $module/src/$dia
    else
      echo "ERROR: don't know how to make $img"
      exit 1
    fi
  done
}

function clean_module {
  local module="$1"
  echo "Cleaning $module"
  rm -f $module/mastercopy/*
}


cd $root
for module in ${modules}; do
  ${cmd}_module ${module}
done
echo SUCCESS


