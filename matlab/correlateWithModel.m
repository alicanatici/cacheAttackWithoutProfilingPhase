
load ../data/attack.dat -ascii

counter = 14433; % found model

[s_box, inv_s_box] = s_box_gen(); %generate the sbox

[row col] = size(attack);

attack = attack((row-4095):row,:);
%fetch the timing measurements and deviation
for i=1:16
    for j=1:256
        attackTiming(i,j) = attack((i-1)*256 + j,7);
        attackDev(i,j) = attack((i-1)*256 + j,8);
    end
end

studyDev = attackDev;  % use the same deviation as attack for the study
resultDimension = 1;

% prepare the calculated study model
for i=16:-1:1
    tmp = bitget(uint16(counter),i);
    if(tmp==1) % hit 
        studyModelRow((16-i)*16+1:(16-i)*16+16) = -1;
    else % miss 
        studyModelRow((16-i)*16+1:(16-i)*16+16) = 1;
    end
end

%apply sbox
for j=1:256
    studyModelRowSbox(s_box(j)+1) = studyModelRow(j);
end

% extend the model to other 16 bytes
for i=1:16
   studyModel(i,:) = studyModelRowSbox; 
end

% calculate the correlation
% same as the Bernstein's
for b=1:16    
    for i=0:255
       c(i+1)=0;
       v(i+1)=0;
       cpos(i+1)=0;
        for j=0:255
            k = bitxor(uint8(i),uint8(j));
            c(i+1) = c(i+1) + studyModel(b,j+1)*attackTiming(b,double(k)+1);
            z = studyDev(b,j+1)*attackTiming(b,double(k)+1);
            v(i+1) = v(i+1) + z*z;
            z = studyModel(b,j+1)*attackDev(b,double(k)+1);
            v(i+1) = v(i+1) + z*z;
        end
    end

    c = abs(c);

    [B, IX] = sort(c,2,'descend');        

    total(b,:) = IX;
    totalC(b,:) = B;

    %caculate the threshold
    temp = 0;
    for k=1:256
        if(B(1) - B(k) < 10*sqrt(v(IX(k))))
            temp = temp+1;
        end
    end
    numok(b) = temp;
end

for b=0:15
    index = mod((b+(16-b*4)),16);
    ordered(index+1,:,resultDimension) = total(b+1,:) - 1 ;
    orderedNumok(resultDimension,index+1) = numok(b+1);
end 

resultDimension = resultDimension+1;

% ordered stores the possible key byte values from most probable to the least probable, 
% for key positions 0 to 15
% orderedNumok stores the total number of key byte values for key positions
% 0 to 15

% format the output as Bernstein's correlate function result
for i=1:16
   resultArray = [orderedNumok(i), i-1, uint8(ordered(i,1:orderedNumok(i)))];
   if(i==1)
       dlmwrite('correlate.txt', resultArray, 'delimiter', '\t','precision', '%02x');
   else
       dlmwrite('correlate.txt', resultArray, '-append', 'delimiter', '\t','precision', '%02x');
   end
end


