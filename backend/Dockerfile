# 1. Build stage
FROM python:3.10-slim AS builder

WORKDIR /app

# Install poetry
RUN pip install poetry

# Copy project definition files from the backend directory
COPY backend/pyproject.toml ./
# The lock file might not exist yet, so make it optional
COPY backend/poetry.lock* ./

# Install dependencies
# Using --no-dev to exclude development dependencies
RUN poetry config virtualenvs.create false && \
    poetry install --no-root --without dev

# 2. Final stage
FROM python:3.10-slim

WORKDIR /app

# Install curl for healthchecks
RUN apt-get update && apt-get install -y curl && rm -rf /var/lib/apt/lists/*

# Create a non-root user
RUN addgroup --system app && adduser --system --group app

# Copy installed packages from the build stage
COPY --from=builder /usr/local/lib/python3.10/site-packages /usr/local/lib/python3.10/site-packages
# Copy executables like uvicorn
COPY --from=builder /usr/local/bin /usr/local/bin

# Copy application code
COPY backend/app/ ./app/

# Set ownership to the app user
RUN chown -R app:app /app

# Switch to the non-root user
USER app

# Expose port
EXPOSE 8000

# Run the application
CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000"]
