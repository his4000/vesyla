
% loop statement testcase (static)

% Features:
%  - deep nested loops

A = [10:20]; %! RFILE<> [0,0]
B = [1:10];  %! RFILE<> [0,0]

for i = 1 : 10
	for j=1 : 2
			A(1) = B(1) + A(1); %! DPU [0,0]
	end
end

for i = 1 : 10
	for j =1 : 2
		for k = 2 : 3
			A(2) = B(2) + A(2); %! DPU [0,0]
		end
	end
end

for i = 1 : 10
	for j = 1 : 2
		for k = 2 : 3
			for l = 1 : 4
				A(3) = B(3) + A(3); %! DPU [0,0]
			end
		end
	end
end
