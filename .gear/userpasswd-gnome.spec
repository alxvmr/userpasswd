Name:    userpasswd-gnome
Version: 0.0.1
Release: alt1

Summary: Graphical utility for changing user password
License: GPLv3
Group:   Other
Url:     https://github.com/alxvmr/userpasswd-gnome

BuildRequires(pre): rpm-macros-cmake
BuildRequires: ccmake gcc-c++
BuildRequires: pkgconfig(gobject-2.0) pkgconfig(gio-2.0) pkgconfig(pam) pkgconfig(pam_misc) pkgconfig(json-glib-1.0)
BuildRequires: pkgconfig(gtk4) pkgconfig(libadwaita-1)

Source0: %name-%version.tar

%description
A graphical utility to change your password in GNOME

%prep
%setup

%build
%cmake
%cmake_build

%install
%cmake_install

%post
chown :shadow %_bindir/pam_helper
chmod g+s %_bindir/pam_helper

%files
%_bindir/userpasswd
%_bindir/pam_helper
%_datadir/applications/userpasswd.desktop
%lang(ru) %_datadir/locale/ru/LC_MESSAGES/userpasswd.mo

%changelog
* Wed Jan 29 2025 Maria Alexeeva <alxvmr@altlinux.org> 0.0.1-alt1
- Initial build for Sisyphus
