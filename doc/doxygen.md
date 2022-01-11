# doxygen related notes

## local build
- install doxygen: [Doxygen Manual: Installation](https://www.doxygen.nl/manual/install.html)  
    Note: you need v1.9.2+ Or theme will not work.
- clone this repo: `git clone https://github.com/libui-ng/libui-ng.git`
- switch dir: `cd libui-ng`
- Clone doxygen theme:
    Or you can follow [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css#installation) installation tutorial.  
    Note: we use [Dark Mode Toggle](https://github.com/jothepro/doxygen-awesome-css#dark-mode-toggle-experimental).
    ```sh
    cd doc/
    git clone https://github.com/jothepro/doxygen-awesome-css
    cd doxygen-awesome-css/
    git checkout v1.6.1
    ```

**windows**
- Open `Doxywizard`
- Load `Doxyfile` settings
- In **"Run"** Tab, press **"Run doxygen"** button

**linux/macOS**
- `doxygen Doxyfile`

The generated html document is located in: `doc/html/`

## doc style

- Comment style: `///`  
    https://www.doxygen.nl/manual/docblocks.html#cppblock
- Some special tags that can be used in comments: https://www.doxygen.nl/manual/commands.html
