# Stage 1: Build the collector
FROM gcc:12.3 as builder

WORKDIR /app

# Install build dependencies (curl)
RUN apt-get update && apt-get install -y libcurl4-openssl-dev

# Copy source code
COPY collector-c/src/ /app/src/
COPY collector-c/Makefile /app/

# Build the collector
RUN make

# Stage 2: Create the final image
FROM debian:bookworm-slim

WORKDIR /app

# Install runtime dependencies (curl)
RUN apt-get update && apt-get install -y libcurl4 && rm -rf /var/lib/apt/lists/*

# Copy the compiled binary from the builder stage
COPY --from=builder /app/collector /app/collector

# Set the entrypoint
ENTRYPOINT ["/app/collector"]

# Default command
CMD ["collector", "--url", "http://backend:8000", "--interval", "5"]
