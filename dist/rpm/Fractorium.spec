%define _topdir       %(pwd)
%define buildroot     %{_topdir}/BUILDROOT

Name:           Fractorium
Version:        26.26.10.1
Release:        1
Summary:        A fractal flame editor with GPU support

Group:          Graphics
BuildArch:      x86_64
License:        GPL
URL:            https://github.com/mfeemster/fractorium
BuildRoot:      %{buildroot}

%description
Fractorium

A fractal flame editor with GPU support.

http://fractorium.com/

The project is maintained on github:

https://github.com/mfeemster/fractorium

News and updates at the above or on Matt Feemster's DeviantArt page:

https://www.deviantart.com/mfeemster


%files
%attr(0755, root, root) "/usr/bin/Fractorium-x86_64.AppImage"
/usr/bin/emberanimate
/usr/bin/embergenome
/usr/bin/emberrender
/usr/bin/fractorium
%attr(0644, root, root) "/usr/share/applications/fractorium.desktop"
%attr(0644, root, root) "/usr/share/fractorium/fractorium.png"

#bench files
%attr(0644, root, root) "/usr/share/fractorium/bench/EmberBench.ps1"
%attr(0644, root, root) "/usr/share/fractorium/bench/golubaja_rippingfrominside_complexcode.flame"
%attr(0644, root, root) "/usr/share/fractorium/bench/mfeemster_basicmemory.flame"
%attr(0644, root, root) "/usr/share/fractorium/bench/tatasz_springcrown_manysimplexforms.flame"
%attr(0644, root, root) "/usr/share/fractorium/bench/tyrantwave_flippeddisc_normal.flame"
%attr(0644, root, root) "/usr/share/fractorium/bench/zy0rg_six_bigcomplexcode.flame"

%changelog

