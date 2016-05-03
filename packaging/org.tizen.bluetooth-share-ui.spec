%define _usrdir	/usr
%define _appdir %{_usrdir}/apps

Name:       org.tizen.bluetooth-share-ui
Summary:    bluetooth share UI application
Version:    0.0.88
Release:    1
Group:      TO_BE_FILLED
License:    Flora-1.1
Source0:    %{name}-%{version}.tar.gz

%if "%{?profile}" != "mobile"
ExcludeArch: %{arm} %ix86 x86_64
%endif

BuildRequires:  pkgconfig(dbus-glib-1)
BuildRequires:  pkgconfig(dlog)
BuildRequires:  pkgconfig(vconf)
BuildRequires:  pkgconfig(appcore-efl)
BuildRequires:  pkgconfig(bundle)
BuildRequires:  pkgconfig(ecore)
BuildRequires:  pkgconfig(edbus)
BuildRequires:  pkgconfig(edje)
BuildRequires:  pkgconfig(elementary)
BuildRequires:  pkgconfig(efl-extension)
BuildRequires:  pkgconfig(evas)
BuildRequires:  pkgconfig(bluetooth-api)
BuildRequires:  pkgconfig(aul)
BuildRequires:  pkgconfig(capi-appfw-application)
BuildRequires:  pkgconfig(capi-network-bluetooth)
BuildRequires:  pkgconfig(bluetooth-share-api)
BuildRequires:  pkgconfig(notification)

BuildRequires:  cmake
BuildRequires:  edje-tools
BuildRequires:  gettext-devel
BuildRequires:  hash-signer

%define PKG_NAME org.tizen.bluetooth-share-ui

%description
bluetooth share UI application


%prep
%setup -q

%build
export CFLAGS="$CFLAGS -DTIZEN_DEBUG_ENABLE"
export CXXFLAGS="$CXXFLAGS -DTIZEN_DEBUG_ENABLE"
export FFLAGS="$FFLAGS -DTIZEN_DEBUG_ENABLE"

export CFLAGS="$CFLAGS -DTIZEN_EFL_THEME_2.2"
export CFLAGS+=" -fpie -fvisibility=hidden"
export LDFLAGS+=" -Wl,--rpath=/usr/lib -Wl,--as-needed -Wl,--unresolved-symbols=ignore-in-shared-libs -pie"

cmake . -DCMAKE_INSTALL_PREFIX=%{_appdir}/org.tizen.bluetooth-share-ui
make %{?jobs:-j%jobs}


%install
rm -rf %{buildroot}
%make_install
PKG_ID=org.tizen.bluetooth-share-ui
%define tizen_sign 1
%define tizen_sign_base /usr/apps/${PKG_ID}
%define tizen_sign_level platform
%define tizen_author_sign 1
%define tizen_dist_sign 1

install -D -m 0644 LICENSE %{buildroot}%{_datadir}/license/org.tizen.bluetooth-share-ui

%post

%postun


#%files
#%defattr(-,root,root,-)

%files
%manifest bluetooth-share-ui.manifest
/etc/smack/accesses.d/org.tizen.bluetooth-share-ui.efl
%defattr(-,root,root,-)
%{_appdir}/org.tizen.bluetooth-share-ui/res/edje/bt-share-layout.edj
%{_appdir}/org.tizen.bluetooth-share-ui/res/edje/images.edj
%{_appdir}/org.tizen.bluetooth-share-ui/bin/bluetooth-share-ui
%{_appdir}/org.tizen.bluetooth-share-ui/author-signature.xml
%{_appdir}/org.tizen.bluetooth-share-ui/signature1.xml
%{_usrdir}/share/icons/default/small/org.tizen.bluetooth-share-ui.png
%{_usrdir}/share/packages/org.tizen.bluetooth-share-ui.xml
%{_datadir}/license/org.tizen.bluetooth-share-ui

