# ifma (I Forgot My Arguments!)
A fast way to record, search for, and run commands that you can't quite remember your arguments for.

## Purpose
As a relatively new Linux user, I often found myself forgetting commands. Commands that had very few arguments and were used the most often weren't an 
issue (come on, do you really think I was forgetting `cd`?). But every couple of weeks or months I would find myself combing through the same
StackExchange questions and ArchWiki pages in order to find the full command I needed. Writing shell scripts helped, but often I would even forget
*those*.

The [tldr pages](https://tldr.sh) are a great way to help with this problem. But in some cases, the examples didn't quite fit my
specific usage case. So, I decided to make a similar application that was customized for my needs.

`ifma` allows you to easily record those commands you often forget, along with a description of what they do. You can easily search through entries
by the command, or by the description.

### Disclaimer
It's possible that this could be used to inhibit learning of Linux commands, especially with `ifma run`. **This is not the intention of this project.**
It is not recommended to use this in a way that would prevent you from understanding what you are doing with each command, such as just copying
and pasting commands blindly and running them whenever you need. I would **not** suggest creating an entry for a command that you do not completely
understand first. However, I *would* suggest leaving yourself detailed descriptions of commands that help you know what each command is doing and what 
the arguments mean. Or you know, at least look at the `man` page I guess.

## Limitations
I wanted to make this tool as easy to use as possible, which meant not using any flags to specify things like commands, arguments, or descriptions.
Because of this, entries created for "add", "all", "remove", and "run" will not be useable. I don't see this being a huge issue for now,
but if there are problems in the future I may go back and add flags for specification.

## Installation
In the project directory, run the following commands:
```
make
sudo make install
```

## Usage
To add an entry for a command (along with any arguments), use:
```
ifma add <command> [<args>]
```
You will then be prompted to enter a description.

It may be especially useful to use this with `history` event designators. For example, to add an
entry for the previously ran command in a shell:
```
ifma add !!
```

To view all entries, use:
```
ifma all
```
This will display a list of entries in the following format:
```
[id] description
[command] [args]
```
The `id` of an entry is implicitly given to each entry. This will be used for removing and running
specific entries.

To view all entries matching a search term, use:
```
ifma <keyword>
```
This will return a list of entries where either the [command] or [description] contains `<keyword>`.
This is mostly used in displaying all entries for a certain command.
It is also worth noting that wrapping`<keyword>` in quotes will allow you to search for a phrase, 
which is useful for searching through descriptions:
```
ifma "<keyword>"
```

To search the entries for a command, as well as the descriptions for that command, use:
```
ifma <command> <keyword>
```
This will only return results with the matching command, and where the description contains
`<keyword>`. Again, you can wrap `<keyword>` in quotes to search for a phrase.

To remove an entry, use:
```
ifma remove <id>
```

To run a command from an entry, use:
```
ifma run <id>
```
**NOTE:** This could be potentially dangerous, and cause irreparable damage to your system. 
**Always** be sure of what the command will do before you run it.
