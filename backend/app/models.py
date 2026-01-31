from sqlalchemy import Column, Integer, Float, String, DateTime, Text, Index, ForeignKey, BigInteger
from sqlalchemy.orm import relationship
from sqlalchemy.sql import func
from app.database import Base


class SystemMetrics(Base):
    """Model for storing system metrics snapshots."""
    __tablename__ = "system_metrics"
    
    id = Column(Integer, primary_key=True, index=True)
    timestamp = Column(DateTime(timezone=True), server_default=func.now(), nullable=False, index=True)
    
    # CPU metrics
    cpu_percent = Column(Float, nullable=False)
    cpu_count = Column(Integer, nullable=False)
    cpu_freq_current = Column(Float, nullable=True)  # MHz
    cpu_freq_min = Column(Float, nullable=True)
    cpu_freq_max = Column(Float, nullable=True)
    
    # Memory metrics
    memory_total = Column(BigInteger, nullable=False)  # bytes
    memory_available = Column(BigInteger, nullable=False)  # bytes
    memory_used = Column(BigInteger, nullable=False)  # bytes
    memory_percent = Column(Float, nullable=False)
    
    # Disk metrics (aggregated for main disk)
    disk_total = Column(BigInteger, nullable=False)  # bytes
    disk_used = Column(BigInteger, nullable=False)  # bytes
    disk_free = Column(BigInteger, nullable=False)  # bytes
    disk_percent = Column(Float, nullable=False)
    
    # Network metrics (aggregated)
    network_bytes_sent = Column(BigInteger, nullable=True)  # bytes
    network_bytes_recv = Column(BigInteger, nullable=True)  # bytes
    
    # System info
    hostname = Column(String(255), nullable=True)
    platform = Column(String(50), nullable=True)
    uptime_seconds = Column(Float, nullable=True)
    
    # GPU metrics
    gpu_count = Column(Integer, default=0)
    gpus = relationship("GpuMetrics", back_populates="metric")
    
    # Index for time-based queries
    __table_args__ = (
        Index('idx_timestamp', 'timestamp'),
    )


class GpuMetrics(Base):
    """Model for storing GPU metrics."""
    __tablename__ = "gpu_metrics"
    
    id = Column(Integer, primary_key=True, index=True)
    metric_id = Column(Integer, ForeignKey("system_metrics.id"), nullable=False)
    
    name = Column(String(255), nullable=False)
    driver_version = Column(String(50), nullable=True)
    memory_total = Column(BigInteger, nullable=False)
    memory_used = Column(BigInteger, nullable=False)
    temperature = Column(Float, nullable=False)
    utilization = Column(Float, nullable=False)
    
    metric = relationship("SystemMetrics", back_populates="gpus")


class SystemLog(Base):
    """Model for storing system logs."""
    __tablename__ = "system_logs"
    
    id = Column(Integer, primary_key=True, index=True)
    timestamp = Column(DateTime(timezone=True), server_default=func.now(), nullable=False, index=True)
    
    level = Column(String(20), nullable=False, index=True)  # INFO, WARNING, ERROR, etc.
    source = Column(String(255), nullable=True)  # Where the log came from
    message = Column(Text, nullable=False)
    log_metadata = Column(Text, nullable=True)  # JSON string for additional data
    
    # Index for filtering
    __table_args__ = (
        Index('idx_timestamp_level', 'timestamp', 'level'),
    )
