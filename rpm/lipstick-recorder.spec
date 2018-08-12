Name:       lipstick-recorder

%{!?qtc_qmake:%define qtc_qmake %qmake}
%{!?qtc_qmake5:%define qtc_qmake5 %qmake5}
%{!?qtc_make:%define qtc_make make}
%{?qtc_builddir:%define _builddir %qtc_builddir}


Summary:    Lipstick recorder client
Version:    0.0.1
Release:    1
Group:      System/GUI/Other
License:    GPLv2
URL:        https://bitbucket.org/jolla/lipstick-jolla-home
Source0:    %{name}-%{version}.tar.bz2
BuildRequires:  pkgconfig(Qt5Core)
BuildRequires:  pkgconfig(Qt5Qml)
BuildRequires:  pkgconfig(Qt5Quick)
BuildRequires:  pkgconfig(Qt5Gui)
BuildRequires:  pkgconfig(wayland-client)
BuildRequires:  qt5-qtwayland-wayland_egl-devel
BuildRequires:  qt5-qtconcurrent-devel

%description
Lipstick recorder client

%prep
%setup -q -n %{name}-%{version}

%build
%qtc_qmake5
%qtc_make %{?_smp_mflags}

%install
rm -rf %{buildroot}
%qmake5_install

%files
%defattr(-,root,root,-)
%attr(755, root, privileged) %{_bindir}/lipstick-recorder
