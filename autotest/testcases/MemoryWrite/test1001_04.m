Y_mem = zeros(1, 16) %! MEM[0,1]
A_reg = [1:16]; %! RFILE[0,0]
Y_reg = zeros(1,16); %! RFILE[1,0]


for i=1:2
    Y_reg = Y_mem(1:16);
		Y_reg = A_reg + Y_reg; %! DPU[0,0]
    Y_mem(1:16)= Y_reg;
end
