# Userpasswd-gnome
> [!WARNING]
> The app is in development

A graphical utility to change your password in GNOME.\
Application are written for use in [ALT](https://getalt.org/en/) distributions.

<p align="center">
    <img src="https://psv4.userapi.com/s/v1/d/Cob6LfOk9A0hinP9Cmrm15Ch5jMap_QengSGRhMKl1ZE2E0mio5Mz8Jyena57II4Qei1GGqafsTCurANjEjHUDpH1cFUb23XyJmlp5UgoWB6zkUzm2dB5g/userpasswd.png">
</p>

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
In the root directory, create a build folder and run `cmake` and `make`:
```bash
mkdir build
cd build
cmake ..
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
.gear/userpasswd-gnome.spec
```

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

