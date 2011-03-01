#!/bin/sh

# Check arguments
if ! test -f "$1" || test $# != "2"; then
    echo "Usage: $0 <image_in.png> <image_out.png>"
    exit 1
fi

echo -n "Converting $1 -> $2: "

# Gimp script interpreter (script_fu, python_fu or tiny_fy)
intr="plug_in_script_fu_eval"

# Script-Fu script.
fu="\
(let* ((image    (car (gimp-file-load RUN-NONINTERACTIVE \"$1\" \"$1\"))) \
       (drawable (car (gimp-image-get-active-layer image))) \
       (color    (car (gimp-image-pick-color image drawable 0 0 0 0 0)))) \
 (gimp-by-color-select drawable color 0 0 0 0 0 0) \
 (gimp-bucket-fill drawable 0 0 100 0 0 0 0) \
 (gimp-selection-invert image) \
 (gimp-selection-feather image 2)
 (gimp-layer-add-alpha drawable) \
 (gimp-edit-copy drawable) \
 (gimp-layer-add-mask drawable (car (gimp-layer-create-mask drawable 4))) \
 (gimp-layer-remove-mask drawable 0) \
 (gimp-convert-rgb image) \
 (gimp-edit-paste drawable 0) \
 (gimp-file-save RUN-NONINTERACTIVE image drawable \"$2\" \"$2\") \
 (gimp-image-delete image))"

gimp -i --batch-interpreter $intr -b "$fu" '(gimp-quit 0)' || exit 1

exit 0
