FROM ubuntu:latest
WORKDIR /home/me/projects/perceptualdiff

# Install required apps.
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    libfreeimage-dev

# Copy all the code to docker.
COPY . .

# Build the app.
RUN make install DESTDIR="/home/me" 

# Start the app which allows args to be passed in.
ENTRYPOINT [ "/home/me/projects/perceptualdiff/docker-entrypoint.sh" ]
