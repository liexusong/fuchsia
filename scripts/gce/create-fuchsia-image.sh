#!/bin/bash
# Copyright 2017 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

if [[ -z $FUCHSIA_GCE_PROJECT ]]; then
  source "$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"/env.sh
fi

$FUCHSIA_DIR/scripts/gce/gce make-fuchsia-image "$@" || exit 1

diskimage="$FUCHSIA_OUT_DIR/$FUCHSIA_GCE_IMAGE.img"

tmp="$(mktemp -d)"
if [[ ! -d $tmp ]]; then
  echo "mktemp failed" >&2
  exit 1
fi
trap "rm -rf '$tmp'" EXIT

cd "$tmp"
mv "$diskimage" disk.raw

tar -Scf "$FUCHSIA_OUT_DIR/$FUCHSIA_GCE_IMAGE.tar" disk.raw
if [ -x "$(command -v pigz)" ]; then
  pigz -f "$FUCHSIA_OUT_DIR/$FUCHSIA_GCE_IMAGE.tar"
else
  gzip -f "$FUCHSIA_OUT_DIR/$FUCHSIA_GCE_IMAGE.tar"
fi
gsutil cp "$FUCHSIA_OUT_DIR/$FUCHSIA_GCE_IMAGE.tar.gz" "gs://$FUCHSIA_GCE_PROJECT/$FUCHSIA_GCE_USER/$FUCHSIA_GCE_IMAGE.tar.gz"
gcloud -q compute images delete "$FUCHSIA_GCE_IMAGE"
gcloud -q compute images create "$FUCHSIA_GCE_IMAGE" --source-uri "gs://$FUCHSIA_GCE_PROJECT/$FUCHSIA_GCE_USER/$FUCHSIA_GCE_IMAGE.tar.gz" --guest-os-features=UEFI_COMPATIBLE
