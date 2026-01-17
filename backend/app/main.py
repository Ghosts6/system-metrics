from fastapi import FastAPI

app = FastAPI(
    title="System Metrics & Log Analytics Platform",
    description="A production-grade service for system monitoring.",
    version="0.1.0",
)

@app.get("/")
def read_root():
    """A simple endpoint to confirm the service is running."""
    return {"status": "ok", "message": "Backend service is running"}

@app.get("/api/v1/health")
def read_health():
    """Health check endpoint for API v1."""
    return {"status": "ok"}
