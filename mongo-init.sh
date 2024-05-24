set -e

mongo <<EOF
db = db.getSiblingDB('archdb')

db.createCollection('trips')
db.trips.createIndex({"id": -1}) 
db.trips.createIndex({"id_owner": -1})

db.createCollection('parties')
db.parties.createIndex({"id": -1}) 
db.parties.createIndex({"id_participant": -1})
EOF