
load ../data/study.dat -ascii

counter = 14433; % found model

[s_box, inv_s_box] = s_box_gen(); %generate the sbox

[row col] = size(study);

study1 = study((row-4095):row,:);
%fetch the timing measurements
for i=1:16
    for j=1:256
        study1timing(i,j) = study1((i-1)*256 + j,7);
    end
end

%apply ýnv sbox 
for i=1:16
    for j=1:256
        study1timingInv(i,inv_s_box(j)+1) = study1timing(i,j);
    end
end

%plot
figure;
for i=1:16
    subplot(8,2,i);
    bar(study1timingInv(i,:));
end

figure;
bar(study1timingInv(13,:));


% plot found model
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

for i=1:16
   studyModel(i,:) = studyModelRowSbox; 
end

%plot 
% figure;
% for i=1:16
%     subplot(8,2,i);
%     bar(studyModel(i,:));
% end

%apply ýnv sbox 
for i=1:16
    for j=1:256
        studyModelInv(i,inv_s_box(j)+1) = studyModel(i,j);
    end
end

%plot
figure;
for i=1:16
    subplot(8,2,i);
    bar(studyModelInv(i,:));
end


