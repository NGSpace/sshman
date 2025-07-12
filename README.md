# SSHMan
When you're too lazy to manage more than 1 ssh server/connection/whatever the fuck you wanna call it.


# Requirements:

You need ncurses... that's about it...

### Fedora Linux:
```sh
sudo dnf install ncurses-devel
```
### Debian and the 900 thousand distros that are based on it:
```sh
sudo apt install libncurses-dev
```
### Other Linux distros
IDFK I am not your dad, chances are that if you are here and reading this then you have enough linux experience to figure this shit out yourself...

### MacOS
Probably possible, not doing it, if you want feel free to send a PR if you figure it out.

### Windows
No.

# Debugging

if you want to run a debug build:
```bash
make clean run
```
or if you want just the debug build:
```bash
make clean buildDebug
```

# Building
```bash
make clean build
```

# Installing
```bash
sudo make clean install
```

<img width="1148" height="678" alt="preview" src="https://github.com/user-attachments/assets/dddf82a1-1521-470c-8fe9-916f9ab3ede4" />
