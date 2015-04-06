-- DROP FUNCTION inbound_forward(character varying);

CREATE OR REPLACE FUNCTION inbound_forward(dst_number character varying)
  RETURNS text AS
$BODY$  
  
DECLARE

  myrec RECORD; 
  id_inboundroute integer; 
 
BEGIN  

	SELECT id INTO myrec FROM inbound_routes WHERE did = dst_number;
	IF NOT FOUND THEN
		RETURN 'ROUTE_NOT_FOUND';
	END IF;
 
	id_inboundroute := myrec.id;
 
	-- SPECIFIC DATE --
	
		SELECT destination INTO myrec
			FROM   inbound_rules
			WHERE  inbound_routes_id = id_inboundroute
				   AND CAST(To_char(current_timestamp, 'HH24:MI:SS') AS TIME) BETWEEN start AND stop
				   AND specific_date = current_date;
		
		-- CASE EXISTS; RETURN DESTINATION
		IF FOUND THEN
			RETURN myrec.destination;
		END IF;
		
	-- END - SPECIFIC DATE --

	-- DAYS --	
	
		SELECT destination INTO myrec
			FROM   inbound_rules 
			WHERE  inbound_routes_id = id_inboundroute
			   AND weekdays LIKE '%' || date_part('dow', current_date) || '%'
			   AND CAST(To_char(current_timestamp, 'HH24:MI:SS') AS TIME) BETWEEN start AND stop;  
		
		IF FOUND THEN
			RETURN myrec.destination;
		END IF;
		
	-- END - DAYS --
 
	-- NO RESULTS
	RETURN 'RULES_NOT_FOUND';  
 
END;  
 
$BODY$
  LANGUAGE plpgsql VOLATILE
  COST 100;
ALTER FUNCTION inbound_forward(character varying)
  OWNER TO ipbx;

