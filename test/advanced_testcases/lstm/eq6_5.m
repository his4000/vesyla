% Eq.[6]
% Original       : H = O + tanh(C)
% Implementation : O = O + tanh(C)

COMPUTING_DRRA_ROW = 2;
COMPUTING_DRRA_COL = 5;
HIDDEN_SIZE = 320;
REG_DEPTH = 32;

C_sram = ones(1, HIDDEN_SIZE);    %! MEM [0,0]
O_sram = ones(1, HIDDEN_SIZE);    %! MEM [0,0]

C_reg_0 = zeros(1, REG_DEPTH);  %! RFILE [0,0]
O_reg_0 = zeros(1, REG_DEPTH);  %! RFILE [1,0]
C_reg_1 = zeros(1, REG_DEPTH);  %! RFILE [0,1]
O_reg_1 = zeros(1, REG_DEPTH);  %! RFILE [1,1]
C_reg_2 = zeros(1, REG_DEPTH);  %! RFILE [0,2]
O_reg_2 = zeros(1, REG_DEPTH);  %! RFILE [1,2]
C_reg_3 = zeros(1, REG_DEPTH);  %! RFILE [0,3]
O_reg_3 = zeros(1, REG_DEPTH);  %! RFILE [1,3]
C_reg_4 = zeros(1, REG_DEPTH);  %! RFILE [0,4]
O_reg_4 = zeros(1, REG_DEPTH);  %! RFILE [1,4]

for i = 1 : HIDDEN_SIZE/(REG_DEPTH)/COMPUTING_DRRA_COL
	C_reg_0 = C_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*5+1:i*REG_DEPTH*COMPUTING_DRRA_COL-4*REG_DEPTH);
	O_reg_0 = O_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*5+1:i*REG_DEPTH*COMPUTING_DRRA_COL-4*REG_DEPTH);
	C_reg_0 = __sl_tanh(C_reg_0); %! DPU [0,0]
	O_reg_0 = O_reg_0 + C_reg_0;  %! DPU [0,0]

	C_reg_1 = C_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*4+1:i*REG_DEPTH*COMPUTING_DRRA_COL-3*REG_DEPTH);
	O_reg_1 = O_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*4+1:i*REG_DEPTH*COMPUTING_DRRA_COL-3*REG_DEPTH);
	C_reg_1 = __sl_tanh(C_reg_1); %! DPU [0,1]
	O_reg_1 = O_reg_1 + C_reg_1;  %! DPU [0,1]

	C_reg_2 = C_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*3+1:i*REG_DEPTH*COMPUTING_DRRA_COL-2*REG_DEPTH);
	O_reg_2 = O_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*3+1:i*REG_DEPTH*COMPUTING_DRRA_COL-2*REG_DEPTH);
	C_reg_2 = __sl_tanh(C_reg_2); %! DPU [0,2]
	O_reg_2 = O_reg_2 + C_reg_2;  %! DPU [0,2]

	C_reg_3 = C_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*2+1:i*REG_DEPTH*COMPUTING_DRRA_COL-1*REG_DEPTH);
	O_reg_3 = O_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*2+1:i*REG_DEPTH*COMPUTING_DRRA_COL-1*REG_DEPTH);
	C_reg_3 = __sl_tanh(C_reg_3); %! DPU [0,3]
	O_reg_3 = O_reg_3 + C_reg_3;  %! DPU [0,3]

	C_reg_4 = C_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*1+1:i*REG_DEPTH*COMPUTING_DRRA_COL-0*REG_DEPTH);
	O_reg_4 = O_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*1+1:i*REG_DEPTH*COMPUTING_DRRA_COL-0*REG_DEPTH);
	C_reg_4 = __sl_tanh(C_reg_4); %! DPU [0,4]
	O_reg_4 = O_reg_4 + C_reg_4;  %! DPU [0,4]

	O_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*5+1:i*REG_DEPTH*COMPUTING_DRRA_COL-4*REG_DEPTH) = O_reg_0;
	O_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*4+1:i*REG_DEPTH*COMPUTING_DRRA_COL-3*REG_DEPTH) = O_reg_1;
	O_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*3+1:i*REG_DEPTH*COMPUTING_DRRA_COL-2*REG_DEPTH) = O_reg_2;
	O_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*2+1:i*REG_DEPTH*COMPUTING_DRRA_COL-1*REG_DEPTH) = O_reg_3;
	O_sram(i*REG_DEPTH*COMPUTING_DRRA_COL-REG_DEPTH*1+1:i*REG_DEPTH*COMPUTING_DRRA_COL-0*REG_DEPTH) = O_reg_4;
end
