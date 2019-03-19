
% loop statement testcase (static)

% Features:
%  - many non-nested loops

A = [10:20]; %! RFILE<> [0,0]
B = [1:10];  %! RFILE<> [0,0]

for i = 1 : 10
	A(1) = B(1) + A(1); %! DPU [0,0]
end

for i = 1 : 10
	A(2) = B(2) + A(2); %! DPU [0,0]
end

for i = 1 : 10
	A(3) = B(3) + A(3); %! DPU [0,0]
end

for i = 1 : 10
	A(i) = B(i) + A(i); %! DPU [0,0]
end

for i = 1 : 10
	A(i) = B(i) + A(i); %! DPU [0,0]
end
