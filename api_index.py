from bs4 import BeautifulSoup
from sys import argv, exit
from os.path import *
from glob import glob
from collections import defaultdict
from argparse import ArgumentParser

parser = ArgumentParser(description='Generate API index for CodeEditor plugin')
parser.add_argument('--input', type=str, default=None, help='Path to the "helpFiles" directory')
parser.add_argument('--output', type=str, default=None, help='Path to the output file')
args = parser.parse_args()

if args is False: exit(1)

if not isdir(args.input):
    print('error: %s is not a directory' % args.input)
    exit(2)

def valid_symbol(sym):
    if ' ' in sym: return False
    if sym.startswith('simx'): return False
    if not sym.startswith('sim'): return False
    return True

def score_url(url):
    if '#' in url: return 1
    return 0

index = defaultdict(set)
htmlFiles = glob(join(args.input, '**', '*.htm'))
for htmlFile in htmlFiles:
    dirName = relpath(normpath(dirname(htmlFile)), args.input)
    relFile = relpath(normpath(htmlFile), args.input)
    print('Parsing %s...' % relFile)
    with open(htmlFile, 'rt', encoding='utf8') as f:
        soup = BeautifulSoup(f.read(), 'html.parser')
        for a in soup.find_all('a', href=True):
            sym = a.text
            if not valid_symbol(sym): continue
            href = a['href']
            if href[0] == '#':
                href = relFile + href
            else:
                href = normpath(join(dirName, a['href']))
            index[sym].add(href)
        for h3 in soup.find_all('h3'):
            a = h3.find('a')
            if not a: continue
            sym = h3.text
            if not valid_symbol(sym): continue
            href = relFile + '#' + a['name']
            index[sym].add(href)
        for table in soup.find_all('table', {'class': 'apiConstantsTable'}):
            h3 = table.find_previous_sibling('h3')
            if not h3: continue
            a = h3.find('a')
            if not a: continue
            href = relFile + '#' + a['name']
            for div in table.find_all('div'):
                sym = div.text.strip()
                if not valid_symbol(sym): continue
                index[sym].add(href)

with open(args.output, 'wt') as f:
    f.write('const char *api_index[] = {\n')
    for k, v in sorted(index.items()):
        v = list(sorted(v, key=score_url))
        f.write('    "%s", "%s",\n' % (k, v[0].replace('\\', '\\\\')))
    f.write('    0L\n')
    f.write('};\n')
