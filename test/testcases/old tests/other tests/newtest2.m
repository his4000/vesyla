t_0 = fi([0.99999999 0.99999999 0.99999999 0.99999999 0.99999999 0.99999999 0.99999999 0.99999999 0.98078528 0.83146961 0.55557023 0.19509032 0.19509032 0.55557023 0.83146961 0.98078528 0.92387953 0.38268343 0.38268343 0.92387953 0.92387953 0.38268343 0.38268343 0.92387953 0.83146961 0.19509032 0.98078528 0.55557023 0.55557023 0.98078528 0.19509032 0.83146961], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [0, 1]

t_1 = fi([0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.55557023 0.98078528 0.19509032 0.83146961 0.83146961 0.19509032 0.98078528 0.55557023 0.38268343 0.92387953 0.92387953 0.38268343 0.38268343 0.92387953 0.92387953 0.38268343 0.19509032 0.55557023 0.83146961 0.98078528 0.98078528 0.83146961 0.55557023 0.19509032], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [1, 1]

x_0 = fi([0.99999999 0.99999999 0.99999999 0.99999999 0.99999999 0.99999999 0.99999999 0.99999999 0.98078528 0.83146961 0.55557023 0.19509032 0.19509032 0.55557023 0.83146961 0.98078528 0.92387953 0.38268343 0.38268343 0.92387953 0.92387953 0.38268343 0.38268343 0.92387953 0.83146961 0.19509032 0.98078528 0.55557023 0.55557023 0.98078528 0.19509032 0.83146961], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [0, 0]

x_1 = fi([0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.55557023 0.98078528 0.19509032 0.83146961 0.83146961 0.19509032 0.98078528 0.55557023 0.38268343 0.92387953 0.92387953 0.38268343 0.38268343 0.92387953 0.92387953 0.38268343 0.19509032 0.55557023 0.83146961 0.98078528 0.98078528 0.83146961 0.55557023 0.19509032], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [1, 0]

X_00 = fi([0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.55557023 0.98078528 0.19509032 0.83146961 0.83146961 0.19509032 0.98078528 0.55557023], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [0, 2]
X_01 = fi([0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.55557023 0.98078528 0.19509032 0.83146961 0.83146961 0.19509032 0.98078528 0.55557023], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [0, 2]
X_10 = fi([0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.55557023 0.98078528 0.19509032 0.83146961 0.83146961 0.19509032 0.98078528 0.55557023], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [1, 2]
X_11 = fi([0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.70710678 0.55557023 0.98078528 0.19509032 0.83146961 0.83146961 0.19509032 0.98078528 0.55557023], 1, 16, 15, 'RoundingMethod', 'Floor'); %! RFILE<> [1, 2]

% dct 1d row by row
for r=1:4
  for c=1:4
    %! RESOURCE_SHARING_BEGIN
    X_00((r-1)*4+c) = sum(x_0((r-1)*8+1:r*8) .* t_0((c-1)*8+1:c*8)); %! DPU [0,0]
    X_01((r-1)*4+c) = sum(x_0((r-1)*8+1:r*8) .* t_1((c-1)*8+1:c*8)); %! DPU [0,2]
    X_10((r-1)*4+c) = sum(x_1((r-1)*8+1:r*8) .* t_0((c-1)*8+1:c*8)); %! DPU [1,0]
    X_11((r-1)*4+c) = sum(x_1((r-1)*8+1:r*8) .* t_1((c-1)*8+1:c*8)); %! DPU [1,2]
    %! RESOURCE_SHARING_END
  end
end

% transpose
for r=1:4
  for c=1:4
    x_0((r-1)*8+c)   = X_00((c-1)*4+r);
    x_0((r-1)*8+c+4) = X_10((c-1)*4+r);
    x_1((r-1)*8+c)   = X_01((c-1)*4+r);
    x_1((r-1)*8+c+4) = X_11((c-1)*4+r);
  end
end

% dct 1d row by row
% for r=1:4
%   for c=1:4
%     %! RESOURCE_SHARING_BEGIN
%     X_00((r-1)*8+c) = sum(x_0((r-1)*8+1:r*8) .* t_0((c-1)*8+1:c*8)); %! DPU [0,0]
%     X_01((r-1)*8+c) = sum(x_0((r-1)*8+1:r*8) .* t_1((c-1)*8+1:c*8)); %! DPU [0,2]
%     X_10((r-1)*8+c) = sum(x_1((r-1)*8+1:r*8) .* t_0((c-1)*8+1:c*8)); %! DPU [1,0]
%     X_11((r-1)*8+c) = sum(x_1((r-1)*8+1:r*8) .* t_1((c-1)*8+1:c*8)); %! DPU [1,2]
%     %! RESOURCE_SHARING_END
%   end
% end
