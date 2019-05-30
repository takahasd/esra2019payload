
%{
ESRA PAYLOAD 2019

"Linear" Kalman filter applied MPU data with timestamped camera
images. Estimates global angles and applies them to camera view to
map the image to its absolute location relative to the payload.

Status: working well with May 28 images and data. Offset in objects from 
image to image is likely caused by movement of the camera.

Requires: data (x,y,z of mag,accel,gyro, timestamps), compensation
constants for magnetometer, and timestamped images in the format
"whatever_XXX.XXX.jpg". No numbers except for timestamp in image names.

Last Updated: May 29, 2019
Author: mitch nelke
%}

clear;
close all;
clc;

% create list of images in the directory
images = dir("May_28/test_img_2/*.jpg");
numImg = length(images);
imTimes = zeros(1,numImg);

% scan image name and replace anything that isn't the timestamp digits with
% '0', then convert image name to an integer.
for k = 1:numImg
    imName = images(k).name;
    flag = false;
    for l = 1:length(imName)
        if isletter(imName(l))
            imName(l) = '0';
        end
        if (imName(l) == '.') && (flag == false)
            flag = true;
        else if imName(l) == '.'
                imName(l) = '0';
            end
        end
        if( imName(l) == '_' )
            imName(l) = '0';
        end
    end
    imTimes(k) = str2num(imName);
end


% magnetic north calculated from 
% https://www.ngdc.noaa.gov/geomag/calculators/magcalc.shtml#igrfgrid
mag_N = [5380,19948,-47356];
mag_N = mag_N / norm(mag_N);

%% compensate magnetometer readings
%{ 
After further investigation, other matlab scripts are available that 
might find the ellipsoid better.
%}


% center of magnetometer readings
cs = [-4.827415618563299;-21.214457273742457;48.391447925934730];

% rotation matrix for magnetometer readings
R = [0.635734280181990,-0.540864903102330,0.550733221799443;0.111989542864533,-0.641294799226053,-0.759077942489840;0.763740968712875,0.544248231120436,-0.347121874320529];

% expanded equation of ellipsoid 
% refer to http://www.scielo.br/scielo.php?script=sci_arttext&pid=S1982-21702015000200329
vf = [7.804234644780049e-04,6.994990948529430e-04,6.157645677612377e-04,5.344178655245771e-13,-2.741207122945212e-13,4.882671449503526e-15,0,0,0];

figure(1);
r_mpu = [1,2,-11]/100; 

% import data with compensated mag
datafilename = "May_28/test_img_2/data.csv";
data = csvread(datafilename);
ts = data(:,10)';

mags_mr_ = data(:,1:3)'; % magnetometer readings
accels_mr_ = data(:,4:6)'; % accelerometer readings
angularVs_mr_ = data(:,7:9)'; % angular velocity readings

% number of frames
n = length(mags_mr_(1,:));

% The accelerometer detects the normal force
% rather than the gravity vector, so the negative of the sensed values are
% used. The gyroscope measures angular rotation of the body but it's easier
% to use the negative angular velocities for the kalman filter because they
% correspond to the movement of the measured vectors rather than the body.
accels_mr = [ -accels_mr_(1,:); -accels_mr_(2,:); -accels_mr_(3,:) ];
angularVs_mr = [ -angularVs_mr_(1,:); -angularVs_mr_(2,:); -angularVs_mr_(3,:) ] * pi / 180;




% apply compensation constants to the magnetometer readings
for k = 1:n
    u = mags_mr_(:,k);
    u = u + cs;
    
    u = R'*u;
    
    u = u ./ [1/sqrt(vf(1)); 1/sqrt(vf(2)); 1/sqrt(vf(3))];
    
    u = R*u;
    
    mags_mr_(:,k) = u;
end


% magnetometer uses different coordinate axes from the accelerometer/gyro
% on the MPU9250, so shift things to the same axes after compensation
mags_mr = [mags_mr_(2,:); mags_mr_(1,:); -mags_mr_(3,:) ];


% plot the magnetometer data
scatter3(mags_mr(1,:), mags_mr(2,:), mags_mr(3,:),'b.');
xlabel('x');
ylabel('y');
zlabel('z');
axis equal;



%{
Angle offsets from accelerometer. Ignore this section for now because the
MPU is kinda loose in the payload for some reason.

theta_a1 = 0.0586;
theta_a2 = 0.0974;
theta_a3 = -0.0964;

rot_ax_a1 = [-.1717,-0.9851,0];
rot_ax_a2 = [0,0,-1];
rot_ax_a3 = [1,0,0];

for k = 1:n
    [w,mags_mr(:,k)] = quat_rot(mags_mr(:,k)',-theta_a2,rot_ax_a2);
    [w,mags_mr(:,k)] = quat_rot(mags_mr(:,k)',-theta_a1,rot_ax_a1);
    
    [w,accels_mr(:,k)] = quat_rot(accels_mr(:,k)',-theta_a3,rot_ax_a3);
    [w,accels_mr(:,k)] = quat_rot(accels_mr(:,k)',-theta_a2,rot_ax_a2);
    [w,accels_mr(:,k)] = quat_rot(accels_mr(:,k)',-theta_a1,rot_ax_a1);
    
    [w,angularVs_mr(:,k)] = quat_rot(angularVs_mr(:,k)',-theta_a2,rot_ax_a2);
    [w,angularVs_mr(:,k)] = quat_rot(angularVs_mr(:,k)',-theta_a1,rot_ax_a1);
end
%}


%% kalman filter

%{
A few simple steps: initialize -> predict -> measure -> update -> predict,
etc.
%}

% estimate the process error of the readings. Probably could be done better
% by fitting recent data to a polynomial and finding the least squares error,
% but we'll use static error for now.

err_mag = .0025; %uT process error magnetometer
err_acc = .0025; %g process error acceleration
err_gyro = .025; %rad/s
err_dgyro = .5; %rad/s^2

% indices
i_a = 1:3; % accelerometer indices
i_g = 4:6; % gyro
i_dg = 7:9; % rate of change of gyro
i_m = 10:12; % magnetometer


% combine error into matrix for kicks
q = [err_acc, err_gyro, err_dgyro, err_mag];


% measurement error. This could be chosen better, but was just eyeballed
% from data that was collected.
error_m = [.1, .25,10, .1];

% initial prediction error is the measurement error
error_p = error_m;

% initial estimation error is also the measurement error
error = error_m;


% set up blank state matrices. For real time calculations in python you
% just need the current and previous states.
state = zeros(12, n);
state_p = zeros(12,1);
state_m = zeros(12,1);
errors = zeros(n,4);

% the initial estimations are just the first measurements
state(i_a, 1) = accels_mr(:,1);
state(i_g, 1) = angularVs_mr(:,1);
state(i_dg, 1) = [0,0,0]';
state(i_m, 1) = mags_mr(:,1);


% now iterate through all the frames of data
for frame = 2:n
    
    % change in time since the last frame
    dt = ts(frame) - ts(frame-1);
    
    
    %% prediction step
    % predict gyro
    % linear prediction: x = x_old + dt * (dx/dt)_old
    state_p(i_g) = state(i_g, frame-1) + dt*state(i_dg,frame-1);
    
    % error in this prediction is also related to the rate of change error
    % and the time since the last frame. q represents process error and
    % prevents the prediction errors from zeroing out.
    
    % prediction error = (prediction error from estimation) + process error
    error_p(2) = error(2) + dt * error(3) + q(2);
    
    state_p(i_dg) = state(i_dg,frame-1);
    error_p(3) = error(3) + q(3);
    
    % predict accelerations
    
    % this is not so linear, but performs better. LKF_iteration() takes a
    % vector and rotates it by many estimations from a constant angular
    % velocity. For best effect, this process could be iterated with a
    % smaller dt_ step size until the result differs less than a desired
    % tolerance from the previous result.
    r0 = state(i_a,frame-1); % accelerometer vector is to be rotated
    dt_ = dt/1000; % step size
    
    % apply the iteration process with a small rate of change in the gyro
    r = LKF_iteration(r0, state(i_g,frame-1),state(i_dg,frame-1)/3,dt_, ts(frame-1),ts(frame));
    
    % final result is the new accelerometer prediction
    state_p(i_a) = r;
    
    % this is not really representative of the actual error, but is an alright
    % approximation that seems to work
    error_p(1) = error(1) + dt*error(2)*error(1) + .5*dt^2*error(3)*error(1) + q(1);
    
    
    % predict magnetometer vector the same way as the acceleromater
    r0 = state(i_m,frame-1);
    dt_ = dt/1000;
    r = LKF_iteration(r0,state(i_g,frame-1),state(i_dg,frame-1)/3,dt_, ts(frame-1),ts(frame));
    state_p(i_m) = r;
    error_p(4) = error(4) + dt*error(2)*error(4) + .5*dt^2*error(3)*error(4) + q(4);
    
    
    %% measurement
    
    % accelerometer data is modified to negate the effects of the rotation
    % of the body of the payload on the acceleration readings. Sometimes it
    % can be significant even though the MPU is just ~10cm from the center.
    state_m(i_a) = accels_mr(:,frame) - cross( angularVs_mr(:,frame), cross( angularVs_mr(:,frame), r_mpu) )';
    state_m(i_g) = angularVs_mr(:,frame);
    state_m(i_m) = mags_mr(:,frame);
    
    % this is not a real measurement but it really helps to keep this as an
    % estimation variable
    state_m(i_dg) = (angularVs_mr(:,frame)-angularVs_mr(:,frame-1)) / dt;
        
    %% kalman gain    
    % kalman gain factor is just the proportion of the prediction error to
    % the sum of the prediction and measurement errors.
    K = error_p ./ (error_p + error_m);
    
    %% update  
    
    % new estimated state is weighted average between predictions and
    % measurements
    state(i_a,frame) = state_p(i_a) + K(1) * (state_m(i_a)-state_p(i_a));
    state(i_g,frame) = state_p(i_g) + K(2) * (state_m(i_g)-state_p(i_g));
    state(i_dg,frame) = state_p(i_dg) + K(3) * (state_m(i_dg)-state_p(i_dg));
    state(i_m,frame) = state_p(i_m) + K(4) * (state_m(i_m)-state_p(i_m));
    
    
    % error update
    error = error_p - K.*error_p;
    
    
    % just saving some intermediate variables for debugging
    errors(frame,:) = error';
    errors_p(frame,:) = error_p;
    
    state_ms(:,frame) = state_m;
    state_ps(:,frame) = state_p;
    
end


%% plotting

% plot magnetometer readings vs estimations
figure(2);
scatter(ts, mags_mr(1,:), 'r.');
hold on;
scatter(ts, mags_mr(2,:), 'b.');
scatter(ts, mags_mr(3,:), 'g.');
plot(ts, state(i_m(1),:), 'r');
plot(ts, state(i_m(2),:), 'b');
plot(ts, state(i_m(3),:), 'g');


% plot where the image times line up on the magnetometer readings
for k = 1:numImg
    plot(imTimes(k), interp1(ts, state(i_m(1),:),imTimes(k)),'ro');
    plot(imTimes(k), interp1(ts, state(i_m(2),:),imTimes(k)),'ro');
    plot(imTimes(k), interp1(ts, state(i_m(3),:),imTimes(k)),'ro');
end
axis([0,ts(end)+1,-2,2]);


% calculate the magnitude of the vectors and plot them
for k = 1:n
    norm_accel(k) = norm(accels_mr(:,k));
    norm_mag(k) = norm(mags_mr(:,k));
    norm_accel_s(k) = norm( state(i_a,k) );
    norm_mag_s(k) = norm( state(i_m,k) );  
end

plot( ts, norm_mag,'c');
plot( ts, norm_mag_s, 'c','LineWidth',2);

plot(ts, state_ps(i_m(1),:), 'r-.');
plot(ts, state_ps(i_m(2),:), 'b-.');
plot(ts, state_ps(i_m(3),:), 'g-.');


% plot the accelerometer data vs the estimated accelerometer values
figure(3);
plot(ts,accels_mr(1,:),'r.');
hold on;
plot(ts,accels_mr(2,:),'b.');
plot(ts,accels_mr(3,:),'g.');

plot(ts, state(1,:), 'r');
plot(ts, state(2,:), 'b');
plot(ts, state(3,:), 'g');
title('accelerations');

plot(ts, norm_accel,'c');
plot(ts, norm_accel_s,'c','LineWidth',2);


% plot the angular velocity measurements vs estimations
figure(4);

plot(ts, angularVs_mr(1,:),'r.');
hold on;
plot(ts, angularVs_mr(2,:),'b.');
plot(ts, angularVs_mr(3,:),'g.');

plot(ts, state(i_g(1),:), 'r');
plot(ts, state(i_g(2),:), 'b');
plot(ts, state(i_g(3),:), 'g');
title('angular velocities vs state');


% plot the angular accelerations vs state
figure(5);
scatter(ts, state_ms(i_dg(1),:),'r.');
hold on;
scatter(ts, state_ms(i_dg(2),:),'b.');
scatter(ts, state_ms(i_dg(3),:),'g.');
plot(ts, state(i_dg(1),:), 'r');
plot(ts, state(i_dg(2),:), 'b');
plot(ts, state(i_dg(3),:), 'g');
title('d_AV data vs state');

%% camera orientation


% usually put a break on this line for debugging to see the graphs from the previous section
close all; 

mag_u = zeros(3,1);
accel_u = zeros(3,1);

% calculate orientation for each image frame
for k = 1:numImg
    
    figure(1);
    clf;
    
    % pull acceleration vector from state estimations via linear
    % interpolation using the camera image timestamp
    accel_u(1) = interp1(ts, state(i_a(1),:), imTimes(k) );
    accel_u(2) = interp1(ts, state(i_a(2),:), imTimes(k) );
    accel_u(3) = interp1(ts, state(i_a(3),:), imTimes(k) );
    
    % pull magnetometer vector as well
    mag_u(1) = interp1(ts, state(i_m(1),:), imTimes(k) );
    mag_u(2) = interp1(ts, state(i_m(2),:), imTimes(k) );
    mag_u(3) = interp1(ts, state(i_m(3),:), imTimes(k) );
    
    
    % make them unit vectors
    accel_u = accel_u / norm(accel_u);
    mag_u = mag_u / norm(mag_u);
    
    
    % first rotation about the X axis to align the gravity vector with XZ
    % plane
    thetaX = atan(accel_u(2)/accel_u(3));
    
    Rx = [  1,0,0;
            0,cos(thetaX),-sin(thetaX);
            0,sin(thetaX),cos(thetaX)];
    
        
    % shift gravity vector using previous rotation
    accel_u1 = Rx*accel_u;
    
    
    % second rotation about the Y axis to align gravity vector with -Z
    % axis. Note the negative sign.
    thetaY = -atan( accel_u1(1)/accel_u1(3) );
    
    Ry = [  cos(thetaY), 0, sin(thetaY);
            0,1,0;
            -sin(thetaY), 0, cos(thetaY)];
    
        
    %{ 
    we just found the rotation to align the gravity vector with the Z axis,
    but what we really needed was the rotation to align the body -Z axis
    with the global -Z axis (gravity). This turns out to be the reverse of
    the Ry*Rx rotation we just calculated.
    %}
    Rxy = (Ry*Rx)';  
    
    
    % rotate accelerometer and magnetometer using newly calculated rotation
    % matrix.
    accel_u2 = Rxy'*accel_u;
    mag_u2 = Rxy'*mag_u;
    
    
    % new local X, Y, -Z coordinates
    zv = Rxy'*[0,0,-2]';
    xv = Rxy'*[1,0,0]';
    yv = Rxy'*[0,1,0]';
    
    % plot the orientation of the current rotation in 3D
    plot3([zv(1),mag_N(1)+zv(1)], [zv(2),mag_N(2)+zv(2)], [zv(3),mag_N(3)+zv(3)],'g','LineWidth',2);
    hold on;
    plot3([zv(1),mag_u2(1)+zv(1)], [zv(2),mag_u2(2)+zv(2)], [zv(3),mag_u2(3)+zv(3)],'g--','LineWidth',1.25);
    plot3([zv(1) accel_u2(1)+zv(1)], [zv(2),accel_u2(2)+zv(2)], [zv(3),accel_u2(3)+zv(3)],'c','LineWidth',2);
    
    % plot the magnetic field vector and the rotated magnetometer vector
    plot3([0,mag_N(1)],[0,mag_N(2)],[0,0],'r','LineWidth',2);
    plot3([0,mag_u2(1)],[0,mag_u2(2)],[0,0],'r--','LineWidth',1.5);
    
    
    % the final rotation is about the Z axis to rotate the tilted local body frame
    % until the magnetometer vector aligns ( in the XY direction only )
    % with the global magnetic field.
    thetaZ = acos( dot( mag_N(1:2)',mag_u2(1:2) )/(norm(mag_N(1:2))*norm(mag_u2(1:2) ) ) );
    
    % use the cross product of the magnetic fields to determine the sign of
    % the rotation angle
    test_u = cross( [mag_N(1:2)';0],[mag_u2(1:2);0] );
    if test_u(3) < 0
        thetaZ = -thetaZ;
    end
    
    Rz = [  cos(thetaZ), -sin(thetaZ), 0;
            sin(thetaZ), cos(thetaZ), 0;
            0,0,1];
    
        
    % perform that rotation
    mag_u3 = Rz'*Rxy'*mag_u;
    accel_u3 = Rz'*Rxy'*accel_u;
    
    % rotate the local body frame using the new Z rotation. The magnetic
    % field vector and the magnetometer reading vector (rotated) should
    % align in the XY plane.
    zv = Rz'*zv;
    xv = Rz'*xv;
    yv = Rz'*yv;
    
    
    % plot that shit.
    plot3([zv(1),mag_N(1)+zv(1)], [zv(2),mag_N(2)+zv(2)], [zv(3),mag_N(3)+zv(3)],'g','LineWidth',2);
    plot3([zv(1),mag_u3(1)+zv(1)], [zv(2),mag_u3(2)+zv(2)], [zv(3),mag_u3(3)+zv(3)],'g--','LineWidth',1.25);
    plot3([zv(1) accel_u3(1)+zv(1)], [zv(2),accel_u3(2)+zv(2)], [zv(3),accel_u3(3)+zv(3)],'c','LineWidth',2);
    
    plot3([zv(1),xv(1)+zv(1)],[zv(2),xv(2)+zv(2)],[zv(3),xv(3)+zv(3)],'b--','LineWidth',.5);
    plot3([0,zv(1)],[0,zv(2)],[0,zv(3)],'b','LineWidth',.5);
    plot3([zv(1),xv(1)+zv(1)],[zv(2),xv(2)+zv(2)],[zv(3),xv(3)+zv(3)],'b--','LineWidth',.5);
    plot3([zv(1),yv(1)+zv(1)],[zv(2),yv(2)+zv(2)],[zv(3),yv(3)+zv(3)],'b-.','LineWidth',.5);
    
    axis([-3,3,-3,3,-3,3]);
    
    % Full rotation to apply to [0,0,-1] to represent the global unit
    % vector for the camera's central axis.
    R_camera = Rz'* Rxy';
    
    
    % angles between the origin and top or side of camera image
    FOV_V = 53.5/2;
    FOV_H = 14.666;
    
    
    % locations of corners of rectangular image
    A = [0,0];
    B = [1,0];
    C = [1,1];
    D = [0,1];
    
    
    % unit vectors representing corners of camera's image
    Z_cs =  sqrt(1-(sind(FOV_H)^2+sind(FOV_V)^2));
    c1 = [sind(FOV_H), sind(FOV_V), -Z_cs];
    c2 = [sind(-FOV_H), sind(FOV_V), -Z_cs];
    c3 = [sind(-FOV_H), -sind(FOV_V), -Z_cs];
    c4 = [sind(FOV_H), -sind(FOV_V), -Z_cs];
    
    
    % rotate camera corner unit vectors to align with global orientation of
    % camera.
    c1_ = R_camera * c1';
    c2_ = R_camera * c2';
    c3_ = R_camera * c3';
    c4_ = R_camera * c4';
    
    % this is just a placeholder for LiDAR readings.
    Z = -2.5;
    
    
    
    % basically, if the algorithm ends up with the camera being aligned
    % upside down then you're fucked and so this just kinda deals with that
    % so it is a little less ugly
    if( c1_(3) > 0 )
        A = -[sign(c1_(1))*30, sign(c1_(2)*30)];
    else
        A = Z / c1_(3) * c1_(1:2);
    end
    
    if( c2_(3) > 0 )
        B = -[sign(c2_(1))*30, sign(c2_(2)*30)];
    else
        B = Z / c2_(3) * c2_(1:2);
    end
    
    if( c3_(3) > 0 )
        C = -[sign(c3_(1))*30, sign(c3_(2)*30)];
    else
        C = Z / c3_(3) * c3_(1:2);
    end
    
    if( c4_(3) > 0 )
        D = -[sign(c4_(1))*30, sign(c4_(2)*30)];
    else
        D = Z / c4_(3) * c4_(1:2);
    end
    
    
    
    % plot the camera image fit to the view of the camera on the ground in
    % 3D space alongside the corners of the camera's vision to help
    % understand what the algorithm thinks the camera is looking at in
    % global space.
    figure(2);
    clf;
    
    plot3([0, A(1)], [0, A(2)], [0, Z], 'g--', 'LineWidth',2);
    hold on;
    plot3([0, B(1)], [0, B(2)], [0, Z], 'g-.', 'LineWidth',2);
    plot3([0, C(1)], [0, C(2)], [0, Z], 'g', 'LineWidth',2);
    plot3([0, D(1)], [0, D(2)], [0, Z], 'g--', 'LineWidth',2);
    plot3( [A(1), B(1)], [A(2), B(2)], [Z Z], 'b-.', 'LineWidth',1 );
    plot3( [B(1), C(1)], [B(2), C(2)], [Z Z], 'g-.', 'LineWidth',1 );
    plot3( [C(1), D(1)], [C(2), D(2)], [Z Z], 'g-.', 'LineWidth',1 );
    plot3( [D(1), A(1)], [D(2), A(2)], [Z Z], 'g-.', 'LineWidth',1 );
    
    axis([-3,3,-3,3,-3,3]);
    grid on;
    
    imName = images(k).name;
    im = imread("May_28/test_img_2/" + imName);
    
    surface([C(1),B(1);D(1),A(1)], [C(2),B(2);D(2),A(2)], [Z,Z;Z,Z], im, 'facecolor', 'texturemap', 'edgecolor', 'none');
    
    view([0,0,3]);
    
    figure(10);
    surface([C(1),B(1);D(1),A(1)], [C(2),B(2);D(2),A(2)], [Z,Z;Z,Z], im, 'facecolor', 'texturemap', 'edgecolor', 'none');
    hold on;
    alpha(0.5);
    
    
    % this is unfinished, but will be used to find the center of an orange
    % block in each image that one is present and plot it as a global
    % location relative to the payload. The deviation of the locations from
    % image to image can be used to quantify the performance of the
    % algorithm and any changes made to it.
    orangePixels = zeros(1000,2);
    i3 = 1;
    for i1 = 1:length(im(:,1,1)-1 ) 
        for i2 = 1:length(im(1,:,1)-1)
            if im(i1,i2,1) > 210 && im(i1,i2,2) > 75 && im(i1,i2,2) < 180
                orangePixels(i3,:) = [i1,i2];
                im(i1,i2,1) = im(i1,i2,1);
                im(i1,i2,2) = im(i1,i2,2);
                im(i1,i2,3) = 0;
                i3 = i3+1;
            
            else
                im(i1,i2,1) = 0;
                im(i1,i2,2) = 0;
                im(i1,i2,3) = 0;
            end
        
        end
    end
    
    y_bar = sum( nonzeros( orangePixels(:,1) ) )/length( nonzeros( orangePixels(:,1) ) );
    x_bar = sum( nonzeros( orangePixels(:,2) ) )/length( nonzeros( orangePixels(:,2) ) );
    
    figure(11);
    clf;
    x_l = length(im(:,1,1));
    y_l = length(im(1,:,1));
    surface([-y_l,y_l;-y_l,y_l], [x_l,x_l;-x_l,-x_l], [0,0;0,0], im, 'facecolor', 'texturemap', 'edgecolor', 'none');
    hold on;
    scatter3(x_bar-y_l/2,y_bar-x_l/2,0,'ro');
    
    
    
    % wait for user input and then continue to next image
    input("");
end
   
    

    

    

