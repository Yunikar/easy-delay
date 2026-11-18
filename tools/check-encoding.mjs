// Fails if a tracked text file is not clean UTF-8.
//
// Catches the three ways this has actually gone wrong:
//   * a file is not valid UTF-8 at all
//   * a file starts with a UTF-8 BOM (shows up as a stray invisible character)
//   * UTF-8 text was decoded as GBK and re-encoded, leaving mojibake that is
//     still valid UTF-8 (a decode check alone does not catch it)
import { readdirSync, readFileSync } from 'node:fs';
import { join, relative } from 'node:path';

const root = process.cwd();
const skipDirs = new Set(['.git', 'build', 'third_party', 'dist', 'node_modules', '.vs']);
const textExt = new Set([
  '.md', '.txt', '.cpp', '.h', '.hpp', '.c', '.cc', '.ps1', '.cmd', '.bat',
  '.mjs', '.js', '.json', '.yml', '.yaml', '.cmake', '.iss',
]);

// Code points that appear when GBK bytes are decoded as UTF-8. Written as
// escapes so this file itself stays ASCII-clean. One is suspicious, several is
// mojibake.
const mojibake =
  /[\uFFFD\u9518\u9225\u6D93\u7039\u93BB\u93C4\u9366\u9428\u95AB\u95C2\u5A34\u93C2\u9359\u93B5\u6769\u95BF\u935A\u59AB\u7ECB\u9477\u7F02\u93C3\u941C\u7ED4\u7EC9\u7F01\u7F03\u7EFE\u7F04]/;

const decoder = new TextDecoder('utf-8', { fatal: true });
const problems = [];

function walk(dir) {
  for (const entry of readdirSync(dir, { withFileTypes: true })) {
    const full = join(dir, entry.name);

    if (entry.isDirectory()) {
      if (!skipDirs.has(entry.name)) walk(full);
      continue;
    }
    if (!entry.isFile()) continue;

    const dot = entry.name.lastIndexOf('.');
    const ext = dot >= 0 ? entry.name.slice(dot) : '';
    const isTrackedText = entry.name === '.gitignore' || textExt.has(ext);
    if (!isTrackedText) continue;

    const rel = relative(root, full);
    const bytes = readFileSync(full);
    if (bytes[0] === 0xEF && bytes[1] === 0xBB && bytes[2] === 0xBF)
      problems.push(`${rel}: starts with a UTF-8 BOM`);

    let text;
    try {
      text = decoder.decode(bytes);
    } catch {
      problems.push(`${rel}: not valid UTF-8`);
      continue;
    }

    const matches = text.match(new RegExp(mojibake.source, 'g'));
    if (matches) problems.push(`${rel}: ${matches.length} mojibake character(s), e.g. U+${matches[0].codePointAt(0).toString(16)}`);
  }
}

walk(root);

if (problems.length > 0) {
  console.error('Encoding check failed:');
  for (const p of problems) console.error('  ' + p);
  process.exit(1);
}

console.log('Encoding check passed: all text files are clean UTF-8.');
