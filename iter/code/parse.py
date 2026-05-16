#!/usr/bin/env python3

# 1. Download input XML: https://www.gutenberg.org/cache/epub/feeds/pgmarc.xml.zip
# 2. Unzip it
# 3. Run parse.py to get books/ directory populated with JSON files
#
# XML format description: https://www.loc.gov/marc/bibliographic/concise/bd100.html

from dataclasses import dataclass
import xml.sax
import json
import random

@dataclass
class Datafield:
  ind1:str = None
  ind2:str = None
  tag:str = None
  subfields:object = None

@dataclass
class Subfield:
  code:str = None
  text:str = None

@dataclass
class Record:
  datafields:str = None


entries = list()
def process(r:Record):
  entry = {
    'author': None,
    'title': None,
    'subjects': [],
    'comments': []
  }
  for df in r.datafields:
    if df.tag == '100':
      author = ' '.join([sf.text for sf in df.subfields])
      entry['author'] = author
    elif df.tag == '245':
      title = [sf.text for sf in df.subfields if sf.code == 'a'][0]
      if title.endswith(': '):
        title = title[:-2]
      entry['title'] = title
    elif df.tag == '500':
      comment = [sf.text for sf in df.subfields if sf.code == 'a'][0]
      entry['comments'].append(comment)
    elif df.tag == '856':
      url = [sf.text for sf in df.subfields if sf.code == 'u'][0]
      entry['comments'].append(f'Available at {url}')
    elif df.tag == '508':
      note = [sf.text for sf in df.subfields][0]
      entry['comments'].append(note)
    elif df.tag == '653':
      subject = [sf.text for sf in df.subfields if sf.code == 'a'][0]
      entry['subjects'].append(subject)
  entries.append(entry)

class MarcHandler(xml.sax.handler.ContentHandler):
  def __init__(self):
    self.record = None
    self.datafield = None
    self.subfield = None

  def startElement(self, name, attrs):
    if name == 'record':
      self.record = Record(
        datafields = list()
      )

    elif name == 'datafield':
      self.datafield = Datafield(
        ind1 = attrs['ind1'],
        ind2 = attrs['ind2'],
        tag = attrs['tag'],
        subfields = list()
      )
    elif name == 'subfield':
      self.subfield = Subfield(
        code = attrs['code'],
        text = ''
      )

  def endElement(self, name):
    if name == 'record':
      process(self.record)
    elif name == 'datafield':
      self.record.datafields.append(self.datafield)
    elif name == 'subfield':
      self.datafield.subfields.append(self.subfield)

  def characters(self, content):
    if self.subfield is not None:
      self.subfield.text += content

handler = MarcHandler()
xml.sax.parse('pgmarc.xml', handler)

rnd = random.Random(17249)
rnd.shuffle(entries)
fno = 0
first = 0
while True:
  last = min(first + rnd.randint(1, 1000), len(entries))
  if last == first:
    break
  with open(f'books-{fno:03d}.jsonl', 'w') as fd:
    for i in range(first, last):
      json.dump(entries[i], fd)
      print('', file=fd)
  first = last
  fno += 1

