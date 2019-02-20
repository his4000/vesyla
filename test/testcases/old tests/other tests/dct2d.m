tf_1_4 = [0:31]; %! RFILE<> [0,0]
tf_5_8 = [0:31]; %! RFILE<> [0,1]

% tf_1_4 = [1.00000000 1.00000000 1.00000000 1.00000000 1.00000000 1.00000000 1.00000000 1.00000000 0.98078528 0.83146961 0.55557023 0.19509032 0.19509032 0.55557023 0.83146961 0.98078528 0.92387953 0.38268343 0.38268343 0.92387953 0.92387953 0.38268343 0.38268343 0.92387953 0.83146961 0.19509032 0.98078528 0.55557023 0.55557023 0.98078528 0.19509032 0.83146961]; %! RFILE<> [0,0]
% tf_5_8 = [0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.55557023 0.98078528 0.19509032 0.83146961 0.83146961 0.19509032 0.98078528 0.55557023 0.38268343 0.92387953 0.92387953 0.38268343 0.38268343 0.92387953 0.92387953 0.38268343 0.19509032 0.55557023 0.83146961 0.98078528 0.98078528 0.83146961 0.55557023 0.19509032]; %! RFILE<> [0,1]

x_1_4 = [0:31]; %! RFILE<> [1,0]
x_5_8 = [0:31]; %! RFILE<> [1,1]

X_1_4 = [0:31]; %! RFILE<> [2,0]
X_5_8 = [0:31]; %! RFILE<> [2,1]


% dct 1d row by row
for row=1:4
  for i=1:4
    X_1_4((row-1)*8+i) = sum(x_1_4((row-1)*8+1:row*8) .* tf_1_4((i-1)*8+1:i*8)); %! DPU [0,0]
    X_1_4((row-1)*8+4+i) = sum(x_1_4((row-1)*8+1:row*8) .* tf_5_8((i-1)*8+1:i*8)); %! DPU [0,1]
    X_5_8((row-1)*8+i) = sum(x_5_8((row-1)*8+1:row*8) .* tf_1_4((i-1)*8+1:i*8)); %! DPU [0,0]
    X_5_8((row-1)*8+4+i) = sum(x_5_8((row-1)*8+1:row*8) .* tf_5_8((i-1)*8+1:i*8)); %! DPU [0,1]
  end
end

% transpose
for row=1:4
  for col=1:4
    x_1_4((row-1)*8+col) = X_1_4((col-1)*8+row);
    x_1_4((row-1)*8+4+col) = X_5_8((col-1)*8+row);
    x_5_8((row-1)*8+col) = X_1_4((col-1)*8+row+4);
    x_5_8((row-1)*8+4+col) = X_5_8((col-1)*8+row+4);
  end
end

% dct 1d col by col
for row=1:4
  for i=1:4
    X_1_4((row-1)*8+i) = sum(x_1_4((row-1)*8+1:row*8) .* tf_1_4((i-1)*8+1:i*8)); %! DPU [0,0]
    X_1_4((row-1)*8+4+i) = sum(x_1_4((row-1)*8+1:row*8) .* tf_5_8((i-1)*8+1:i*8)); %! DPU [0,1]
    X_5_8((row-1)*8+i) = sum(x_5_8((row-1)*8+1:row*8) .* tf_1_4((i-1)*8+1:i*8)); %! DPU [0,0]
    X_5_8((row-1)*8+4+i) = sum(x_5_8((row-1)*8+1:row*8) .* tf_5_8((i-1)*8+1:i*8)); %! DPU [0,1]
  end
end
