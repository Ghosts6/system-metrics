# Project Setup Guide

This document provides instructions on how to set up, run, and test the System Metrics project using Docker.

## 1. Prerequisites

Before you begin, ensure you have the following installed on your system:

- **Docker**: [Installation Guide](https://docs.docker.com/get-docker/)
- **Docker Compose**: [Installation Guide](https://docs.docker.com/compose/install/)

## 2. Docker Setup

The entire backend environment, including the database and cache, is orchestrated using Docker Compose.

### Step 1: Environment Variables

The services require environment variables for configuration. These are managed in a `docker.env` file in the project root.

1.  Create a file named `docker.env` in the root of the project.
2.  Add the following configuration. You can change the `POSTGRES_USER` and `POSTGRES_PASSWORD` if you wish, but ensure they match between the `db` and `backend` sections.

    ```env
    # PostgreSQL Database
    POSTGRES_DB=system_metrics
    POSTGRES_USER=user
    POSTGRES_PASSWORD=password

    # Backend Application
    # This URL is used by the application to connect to the database.
    # The hostname 'db' matches the service name in docker-compose.yml.
    DATABASE_URL=postgresql://user:password@db:5432/system_metrics

    # Redis Cache
    # The hostname 'redis' matches the service name in docker-compose.yml.
    REDIS_HOST=redis
    REDIS_PORT=6379

    # A placeholder for application secrets
    SECRET_KEY=replace_this_with_a_real_secret_key
    ```
    *(Note: This file should already exist if you've followed the setup steps).*

### Step 2: Build and Run Services

From the project's root directory, run the following command to build the container images and start the services in detached mode:

```sh
sudo docker-compose up -d --build
```

### Step 3: Verify Services

To check if all services are up and running, use:

```sh
sudo docker-compose ps
```

You should see the `backend`, `db`, and `redis` services with a status of `Up` or `running`.

The backend API will be accessible at `http://localhost:8000`. You can test this by opening the URL in your browser or using a tool like `curl`.

To view the logs for a specific service:
```sh
sudo docker-compose logs <service_name>
# Example for backend:
sudo docker-compose logs backend
```

## 3. Running Backend Tests

The backend tests are written using the `pytest` framework and are located in the `backend/tests/` directory.

To provide a consistent testing environment without polluting the production image with development tools, we use a dedicated `test` service.

From the project's root directory, run the following command:

```sh
sudo docker-compose run --rm test
```

This command does the following:
- `docker-compose run`: Runs a one-off command on a service. It will start the service's dependencies (`db` and `redis`) if they are not already running.
- `--rm`: Removes the container after the tests are complete, keeping your system clean.
- `test`: The name of the service to run, which is configured in `docker-compose.yml` to use `backend/Dockerfile.dev` and run `pytest` by default.

You should see a new container being built (the first time) and then the `pytest` test results printed to your console.
