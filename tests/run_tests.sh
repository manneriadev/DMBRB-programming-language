#!/usr/bin/env bash
# Usage: tests/run_tests.sh [--update]
#   --update  regenerate .expected files from current output (review them!)
cd "$(dirname "$0")/.." || exit 1

BIN=./dmbrb
TMP=build/test_tmp
mkdir -p "$TMP"
case "$(uname -s)" in MINGW*|MSYS*|CYGWIN*) EXT=".exe" ;; *) EXT="" ;; esac
UPDATE=0
[ "${1:-}" = "--update" ] && UPDATE=1

pass=0
fail=0
ok()   { echo "  ok    $1"; pass=$((pass+1)); }
bad()  { echo "  FAIL  $1  ($2)"; fail=$((fail+1)); }

echo "== tests =="
for src in tests/cases/*.dmb; do
  [ -e "$src" ] || continue
  name=$(basename "$src" .dmb)
  exe="$TMP/$name$EXT"
  rm -f "$exe"

  if [[ $name == fail_* ]]; then
    if "$BIN" "$src" -o "$TMP/$name" >/dev/null 2>&1; then
      bad "$name" "should not compile"
    else
      ok "$name"
    fi
    continue
  fi

  if ! "$BIN" "$src" -o "$TMP/$name" >/dev/null 2>"$TMP/$name.err"; then
    bad "$name" "compilation failed"
    continue
  fi

  actual=$(timeout 10 "$exe" </dev/null 2>&1 | tr -d '\r')
  expected_file="tests/cases/$name.expected"

  if [ $UPDATE -eq 1 ]; then
    printf '%s\n' "$actual" > "$expected_file"
    echo "  updated $expected_file"
    continue
  fi

  if [ ! -f "$expected_file" ]; then
    bad "$name" "no .expected file, run with --update"
  elif [ "$actual" == "$(tr -d '\r' < "$expected_file")" ]; then
    ok "$name"
  else
    bad "$name" "output differs"
  fi
done

echo "== examples compile =="
for src in examples/*.dmb; do
  [ -e "$src" ] || continue
  name=$(basename "$src" .dmb)
  if "$BIN" "$src" -o "$TMP/ex_$name" >/dev/null 2>&1; then ok "$name"; else bad "$name" "compilation failed"; fi
done

echo
echo "passed: $pass, failed: $fail"
[ $fail -eq 0 ]
