
% Calculation with accumulation testcase

% Features:
%  - Multiple MAC operations inside loop using multiple DPUs

tf_1_4 = [0:31]; %! RFILE<> [0,0]
tf_5_8 = [63:32]; %! RFILE<> [0,1]

x_1_4 = [0:31]; %! RFILE<> [1,0]
x_5_8 = [31:0]; %! RFILE<> [1,1]

X_1_4 = zeros(1,32); %! RFILE<> [0,2]
X_5_8 = zeros(1,32); %! RFILE<> [1,2]

for row=1:4
	for i=1:4
		X_1_4((row-1)*8+i) = sum(x_1_4((row-1)*8+1:row*8) .* tf_1_4((i-1)*8+1:i*8)); %! DPU [0,0]
		X_1_4((row-1)*8+4+i) = sum(x_1_4((row-1)*8+1:row*8) .* tf_5_8((i-1)*8+1:i*8)); %! DPU [0,1]
%		X_5_8((row-1)*8+i) = sum(x_5_8((row-1)*8+1:row*8) .* tf_1_4((i-1)*8+1:i*8)); %! DPU [0,0]
%		X_5_8((row-1)*8+4+i) = sum(x_5_8((row-1)*8+1:row*8) .* tf_5_8((i-1)*8+1:i*8)); %! DPU [0,1]
	end
end
