# DictOfYourOwn
Organizational program for managing conlang vocabulary.

Terminal (atomic) commands:
help: display this menu
exit: quit program
export [FILENAME]: export this dictionary as [FILENAME].txt
import [FILENAME] ?[PATH]: import [FILENAME].txt as an entry. For extern files, provide a path.
importall ?[DIRECTORY]: import all txt files in [DIRECTORY], or this one
remove [FILENAME]: remove the entry [FILENAME]
rename [FILENAME] [NEWNAME]: rename the entry [FILENAME] to [NEWNAME]
save: save to disk, although this is done automatically on exit
view [FILENAME]: display entry definition and associated tags

Chainable commands (first one generates stream from all entries):
desc -secr [STRING]: return all entries with description matching string [STRING] and flags
list -r [TAG]: return all entries with tag [TAG]
search -secr [STRING]: return all entries with name matching string [STRING] and flags
sort -alr: sort stream [a] alphanumerically, [l] by length, [r] in reverse

Searching flags: [s] startswith, [e] endswith, [c] contains, [r] inverse (non-matching)
Flags must always precede the argument. Spaces must be substituted by the \ (backslash) character.

Set operators & (intersection) and | (union) may be used between streams
All operations are done from left to right. A set operator begins a new stream for
its right-hand value. For example, looking up all entries with tag 'abstract' or tag
'concrete' that do not contain 'the' in their name, sorted by length, could be done like this:
>sort -l list concrete | sort -l list abstract & sort -l search -cr the
