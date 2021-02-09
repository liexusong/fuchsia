#!/usr/bin/env python3
# Copyright 2020 The Fuchsia Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

import argparse
import datetime
import os
import platform
import re
import subprocess
import sys

from typing import List, Callable

from common import *
from identifiers import IDENTIFIERS
from styles import STYLES
from uses import USES


# Validate IDENTIFIERS
# check that the style & use names in deny rules are valid
def validate_identifiers():
    style_names = frozenset(style.name for style in STYLES)
    use_names = frozenset(use.name for use in USES)
    for ident in IDENTIFIERS:
        for deny in ident.deny:
            for style in deny.styles:
                if style not in style_names:
                    print(
                        f'Unknown style name "{style}" in deny list for "{ident.name}"'
                    )
                    sys.exit(1)
            for use in deny.uses:
                if use not in use_names:
                    print(
                        f'Unknown use name "{use}" in deny list for "{ident.name}"'
                    )
                    sys.exit(1)


validate_identifiers()


def generated(prefix: str) -> str:
    """Return a header line indicating that this is a generated file."""
    return """{prefix} Copyright 2019 The Fuchsia Authors. All rights reserved.
{prefix} Use of this source code is governed by a BSD-style license that can be
{prefix} found in the LICENSE file.
{prefix} Generated by {generator}.
""".format(
        prefix=prefix,
        generator='//src/tests/fidl/dangerous_identifiers/generate')


def generate_fidl(
        fidl_file: str, style: Style, use: Use, library_name: str,
        identifier_defs: List[Identifier]):
    idents = [ident.scoped(style, use) for ident in identifier_defs]
    with open(fidl_file, 'w') as f:
        f.write(generated('//'))
        f.write('library %s;\n' % library_name)
        use(f, [ident for ident in idents if not ident.denied])


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--use', required=True, choices=[u.name for u in USES])
    parser.add_argument(
        '--style', required=True, choices=[s.name for s in STYLES])
    parser.add_argument('--shards', required=True)
    parser.add_argument('--shard', required=True)
    parser.add_argument('--fidl-library', required=True)
    parser.add_argument('--out', required=True)
    args = parser.parse_args()

    style = next(s for s in STYLES if s.name == args.style)
    use = next(u for u in USES if u.name == args.use)

    shards = args.shards.split(',')
    assert args.shard in shards
    num_shards = len(shards)
    shard_num = shards.index(args.shard)

    identifiers = [
        identifier for index, identifier in enumerate(IDENTIFIERS)
        if (index % num_shards) == shard_num
    ]

    generate_fidl(args.out, style, use, args.fidl_library, identifiers)