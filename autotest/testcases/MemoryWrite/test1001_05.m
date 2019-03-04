Y_mem = zeros(1, 32) %! MEM[0,0]
A_reg = [1:32]; %! RFILE[0,0]
Y_reg = zeros(1,32); %! RFILE[0,0]


for i=1:2
    Y_reg = Y_mem(1:32);
		Y_reg = A_reg + Y_reg; %! DPU[0,0]
    Y_mem(1:32)= Y_reg;
end
