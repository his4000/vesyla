
% FFT

MN = 32; % Number of FFT data
N = 16; % Number of FFT points
STAGE_NO = 3; % Stages
MEM_BLOCK_NO = 2; % memory blocks
r = 0; % initial row
c = 0; % initial column

MDr = [1:MN]; %! MEM<>[r,c]
MDi = [1:MN]; %! MEM<>[r,c]
MWr = [1:MN/2]; %! MEM<>[r,c]
MWi = [1:MN/2]; %! MEM<>[r,c]
Dr = [1:N]; %! RFILE<>[r,c]
Di = [1:N]; %! RFILE<>[r+1,c]
Wr = [1:N/2]; %! RFILE<>[r,c+1]
Wi = [1:N/2]; %! RFILE<>[r,c+1]

Tr0 = zeros(1, N/2); %! CDPU<mod=4,out=0>[r,c]
Tr1 = zeros(1, N/2); %! CDPU<mod=4,out=1>[r,c]
Ti0 = zeros(1, N/2); %! CDPU<mod=4,out=0>[r,c+1]
Ti1 = zeros(1, N/2); %! CDPU<mod=4,out=1>[r,c+1]

Temp1 = [0]; %! TEMP_VAR
Temp2 = [0]; %! TEMP_VAR
Temp3 = [0]; %! TEMP_VAR
Temp4 = [0]; %! TEMP_VAR

for b = 0 : MEM_BLOCK_NO-1

	mr = bitsll(b, N); %! RACCU_VAR<mem_address>
	mr0 = mr + 1; %! RACCU_VAR
	mr1 = mr0 + N; %! RACCU_VAR<mem_address>

	Dr = MDr(mr0 : mr1);
	Di = MDi(mr0 : mr1);
	Wr = MWr(mr0 : mr1);
	Wi = MWi(mr0 : mr1);

	for stage = 1 : STAGE_NO

		st 		  = stage + 0; %! RACCU_VAR
		r0        = bitsll(1, st); %! RACCU_VAR
		half_size = bitsra(r0, 1); %! RACCU_VAR
		block_no  = bitsra(N, st); %! RACCU_VAR
		bn0		  = block_no - 1; %! RACCU_VAR

		for i = 0 : bn0

			twi	= 1; %! RACCU_VAR
			ri = bitsll(i, st); %! RACCU_VAR

			for j = 1 : half_size
				
				k = ri + j; %! RACCU_VAR
				kh = k + half_size; %! RACCU_VAR

				%! RESOURCE_SHARING_BEGIN

				Tr0(k) = Dr(k) + Dr(kh) .* Wr(twi);
				Tr1(k) = Dr(k) - Dr(kh) .* Wr(twi);

				Ti0(k) = Di(k) + Di(kh) .* Wr(twi);
				Ti1(k) = Di(k) - Di(kh) .* Wr(twi);

				Temp1 = Tr1(k) + Di(kh) .* Wi(twi); %! DPU<mod=5,out=0>[r+1,c]
				Temp2 = Tr0(k) - Di(kh) .* Wi(twi); %! DPU<mod=5,out=1>[r+1,c]

				Temp3 = Ti0(k) + Dr(kh) .* Wi(twi); %! DPU<mod=5,out=0>[r+1,c+1]
				Temp4 = Ti1(k) - Dr(kh) .* Wi(twi); %! DPU<mod=5,out=1>[r+1,c+1]

				Dr(kh) = Temp1;
				Dr(k)  = Temp2;
				Di(k)  = Temp3;
				Di(kh) = Temp4;

				%! RESOURCE_SHARING_END

				twi = twi + block_no;
			end
		end
	end
end

