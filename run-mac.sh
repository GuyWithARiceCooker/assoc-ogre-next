#!/usr/bin/env bash
# assoc-next (Ogre-Next): a `build/` könyvtárból indul, mert a `plugins.cfg`, `ogre-next.cfg` és a log
# a futtatható munkakönyvtárához képest értelmeződik (ugyanaz a elv, mint egy egyszerű Ogre mintánál).
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT/build"
exec ./assoc-next
