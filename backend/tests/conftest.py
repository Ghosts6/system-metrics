import os
import pytest

# Set environment variables BEFORE importing app modules
# Use SQLite for testing to avoid PostgreSQL dependency issues
os.environ["DATABASE_URL"] = "sqlite:///./test_system_metrics.db"
os.environ["REDIS_HOST"] = "localhost"
os.environ["REDIS_PORT"] = "6379"

from sqlalchemy import create_engine
from sqlalchemy.orm import sessionmaker
from fastapi.testclient import TestClient
from app.database import Base, get_db
from app.main import app


@pytest.fixture(scope="function")
def db_session():
    """Create a test database session."""
    # Create test database
    engine = create_engine("sqlite:///./test_system_metrics.db", connect_args={"check_same_thread": False})
    Base.metadata.create_all(bind=engine)
    TestingSessionLocal = sessionmaker(autocommit=False, autoflush=False, bind=engine)
    
    # Create session
    db = TestingSessionLocal()
    try:
        yield db
    finally:
        db.close()
        # Clean up
        Base.metadata.drop_all(bind=engine)
        # Remove test database file
        if os.path.exists("./test_system_metrics.db"):
            os.remove("./test_system_metrics.db")


@pytest.fixture(scope="function")
def client(db_session):
    """Create a test client with database override."""
    def override_get_db():
        try:
            yield db_session
        finally:
            pass
    
    app.dependency_overrides[get_db] = override_get_db
    yield TestClient(app)
    app.dependency_overrides.clear()
