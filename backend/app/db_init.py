from app.database import engine, Base
from app.models import SystemMetrics, SystemLog

def init_db():
    """Initialize database tables."""
    print("Dropping all database tables...")
    Base.metadata.drop_all(bind=engine)
    print("✓ Database tables dropped successfully")
    print("Creating database tables...")
    Base.metadata.create_all(bind=engine)
    print("✓ Database tables created successfully")

if __name__ == "__main__":
    init_db()
