@echo off
dxc meshlet_as.hlsl -T as_6_6 -E as_main -Fo ../bin/meshlet_as.dxil
dxc meshlet_ms.hlsl -T ms_6_6 -E ms_main -Fo ../bin/meshlet_ms.dxil
dxc meshlet_ps.hlsl -T ps_6_6 -E ps_main -Fo ../bin/meshlet_ps.dxil