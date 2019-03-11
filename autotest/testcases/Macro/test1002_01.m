
% General testcase

% Features:
%  - Simple vector initialization and scalar addition
%  - (All variables and operations allocated to one cell.)

A = [3 : 12]; %! RFILE<> [0,0]
B = [5 : 14]; %! RFILE<> [0,0]
C = zeros(1,5);      %! RFILE<> [0,0]

{% for i in range(x) %}
C({{i}}+1) = A({{i}}+1) + B({{i}}+1); %! DPU [0,0]
{% endfor %}
