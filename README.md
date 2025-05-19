# userpasswd

A graphical utility to change your password in GNOME.\
Application are written for use in [ALT](https://getalt.org/en/) distributions.

The application is presented in two interfaces: Adwaita and only GTK4

<p align="center">
    <img src="https://psv4.userapi.com/s/v1/d/8nCNxSIJQC4mnCC21-MuwDHanu2l7hfpvQ3dydpZTDPOyKkjqYK3Go3Vq2g7dN-RUkIlBnQU6_xK5FE-gj_ueWw2gjRNd35WRjcLJM0yGsTP0Kcwx7SlgQ/Group_1_1.png">
</p>

These interfaces are provided in the **userpasswd-gnome** and **userpasswd** packages, respectively. 
The local build is described below.

# Dependencies
```
cmake
gcc
gobject-2.0
gio-2.0
pam_misc
json-glib-1.0
gtk4
libadwaita-1
```

# Build
## Local build
In the root directory, create a build folder and run `cmake` and `make`.\
To build the version with **Adwaita**:
```bash
mkdir build
cd build
cmake -DUSE_ADWAITA=ON ..
make
make install
```
To build the version with **only GTK4**:
```bash
mkdir build
cd build
cmake -DUSE_ADWAITA=OFF ..
make
make install
```
After that you need to manually assign the shadow group and SGID bit to `pam_helper`:
```bash
chown :shadow /bin/pam_helper
chmod g+s /bin/pam_helper
```
## RPM package
The `.spec` file for the project is in the repository:
```
.gear/userpasswd.spec
```
After the RPM package is built, the following packages will be created:
* `userpasswd-common-<version>-<release>.<arch>.rpm` - provides common files for userpasswd and userpasswd-gnome (pam_helper, desktop file, icons, translations, ...);
* `userpasswd-<version>-<release>.<arch>.rpm` - provides an application binary with only GTK4 version;
* `userpasswd-gnome-<version>-<release>.<arch>.rpm` - provides an application binary with Adwaita version.

> [!WARNING]
> The `userpasswd-common` package is required for the rest of the packages to work

> [!WARNING]
> If `userpasswd` and `userpasswd-gnome` packages are installed on the system at the same time, the binary from `userpasswd-gnome` will be used thanks to the alternatives mechanism

# Application Architecture
Userpasswd is used to change a user's password.\
To change the password it uses its own module `pam_helper`, wich creates a context and initiates a PAM transaction. The `passwd` is passed as a service and the user is the user for whom the session started.

<p align="center">
    <img src="https://psv4.userapi.com/s/v1/d/OL2OjH6kDb7LZv0xTcwrZpjxcnEfOd8QzbOQbOELldShQomT4YcNRpscIHeIS64UjYTo0P4kWogxyhvN4eO08RaNMYXzxwTEe4rbfWsGvsO0EsqA5SwvZg/userpasswd.png">
</p>

## Components
**pam_helper**:
- Gets strings from PAM module (`passwd`)
- Translates to JSON and sends the request to `userpasswd`
- Receives a JSON response from `userpasswd`, parses and sends the response to `passwd`

**userpasswd:**
- GUI application
- Requests (and renders) user data in the GUI based on the `pam_helper` request
- Based on the entered user data? generates JSON and passes it to `pam_helper`

## Inter-process data transfer format (JSON)

`pam_helper` can give the following JSON to retrive data.
The json specifies a `type` key that tells whether `pam_helper` expects any data:
- `input` - data should be sent according to the sent template
- `output` - no data should be transmitted, it is just information about the process operation

Example:
- Input type
```json
{"current_password":null,"type":"input"}
```
```json
{"new_password":null,"type":"input"}
```
What data should be passed back (for example, for `current_password`):
```json
{"current_password":"pa$$word"}
```
- Output type

There are 2 JSON senders with `type="output"`: `pam_conv` and `pam_status`. The JSON structure for each of them specific:
```json
{"pam_conv_mess":"Changing password for user.","type_content":"pam_conv","type":"output"}
```
```json
{"pam_status_code":7,"pam_status_mess_en":"Authentication failure","pam_status_mess_ru":"Сбой аутентификации","type_content":"pam_status","type":"output"}
```

