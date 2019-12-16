# OperatingSystem-jos
 jos lab, MIT 6.828
- You can switch to branch 'labx' to see the implementation of each stage.
- Each lab's implementation and contribution are shown in corresponding branch.
## Requirement
### Operating Environment
  - I recommend you to use ubuntu-16.04.2(32-bit) to run this operating system.
  - You can use the virtual machine provided by us.
    ```
        wget http://ftp.sjtu.edu.cn/ubuntu-cd/16.04.2/ubuntu-16.04.2-desktop-i386.iso
    ```
### QEMU
  - Please follow the instructions to build a correct QEMU:   
    ```
        cd ~
        wget http://ipads.se.sjtu.edu.cn/courses/os/tools/qemu-1.5.2.tar.gz
        tar xf qemu-1.5.2.tar.gz
    ```
  - On Linux, you may need to install the SDL development libraries to get a graphical VGA window. On Debian/Ubuntu, this is the libsdl1.2-dev package. You may also need basic building tools
    ```
        sudo aptitude install build-essential autoconf libtool pkg-config libglib2.0-dev libfdt-dev
    ```
  - Configure the source code:
    - Linux:
    ```
        cd qemu-1.5.2
        ./configure --prefix=/usr/local --target-list="i386-softmmu"
    ```
    - OS X:
    ```
        cd qemu-1.5.2
        ./configure --disable-sdl [--prefix=PFX] --target-list="i386-softmmu"
    ```
  - Run make && make install
    ```
        make
        sudo make install
    ```
## Test
- Switch to branch 'labX' if you want to test labX's implementation.
- Please ‘make’ in the directory before you run the testing scripts.
- Run testing scripts 'grade-labX'.
  ```
    $ git checkout labX 
    $ cd labX
    $ make
    $ ./grade-labX
  ```
